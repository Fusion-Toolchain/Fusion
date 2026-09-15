#include <Fusion/IRTypes/HidrType.h>
#include <Fusion/FusionTypes.h>
#include <Internal/Backend/Fus_Backend.h>
#include <Internal/Memory/Fus_Arena.h>
#include <Fusion/Fusion.h>

#include <Internal/Fus_TraceTree.h>

// BACKEND INTERFACE
#include <BackendInterface/Backend.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// LOCAL
#include "x86_familys.h"
#include "x86_functions.h"
#include "x86_types.h"
#include "x86_helpers.h"

#include "InstructionSets/x86_instructions.h" // IMPORT CASES
#include "x86_FamilyDefine.inc"

static inline void X86_MountPrefixHidr(const FusHidrNode_t* mir_node, x86Instruction_t* instr)
{
    // limpa estado
    instr->has_prefix = false;
    instr->prefix.prefix_size = 0;

    // operand override (16-bit em modo 32/64)
    if (mir_node->op_size == HIDR_OP_SIZE_16) {
        instr->prefix.prefix[instr->prefix.prefix_size++] = 0x66;
        instr->has_prefix = true;
    }
}

static inline void X86_MountRex(const FusHidrNode_t* mir_node, x86Instruction_t* instr)
{
    instr->has_rex = false;

    // W: operação 64-bit
    if (mir_node->op_size == HIDR_OP_SIZE_64 &&
    !(mir_node->src.type == HIDR_OPERAND_TYPE_IMM &&
    mir_node->src.data.imm.size == HIDR_IMM8)) {
        instr->rex.w = 1;
        instr->has_rex = true;
    }

    // R: extensão do campo reg (dst normalmente)
    if (mir_node->dst.type == HIDR_OPERAND_TYPE_REG) {
        size_t idx = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
        if (idx != (size_t)-1 && idx >= 8) {
            instr->rex.r = 1;
            instr->has_rex = true;
        }
    }

    // B: extensão do campo rm (src normalmente)
    if (mir_node->src.type == HIDR_OPERAND_TYPE_REG) {
        size_t idx = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->src.data.reg));
        if (idx != (size_t)-1 && idx >= 8) {
            instr->rex.b = 1;
            instr->has_rex = true;
        }
    }

    // memória (base register)
    if (mir_node->src.type == HIDR_OPERAND_TYPE_MEM_REF) {
        size_t idx = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->src.data.memory_ref.base));
        if (idx != (size_t)-1 && idx >= 8) {
            instr->rex.b = 1;
            instr->has_rex = true;
        }
    }
}

static inline bool X86_SelectFamily(FusBackendApi_t* FUS, X86BackendContext* backend_ctx)
{
    FusTraceTree trace = NULL;
    FUSB_GET_TRACE_FUSION(FUS,&trace);
    char buffer[256];

    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    for (size_t i = 0; i < (sizeof(familys) / sizeof(familys[0])); i++) {
        if (familys[i].opcode != mir_node->opcode) continue;

        for (size_t j = 0; j < familys[i].rule_count; j++) {
            X86FamilyRule_t* rules = &familys[i].rules[j];
            if (rules->dst_type != mir_node->dst.type) continue;
            if (rules->src_type != mir_node->src.type) continue;
            if (rules->builder(backend_ctx)) {
                snprintf(buffer, sizeof(buffer),
                    "Backend HIDR Family Lookup, Opcode=%u",
                    mir_node->opcode);

                FUS_PUSH_ERR(trace,FUSION_OK,buffer);
                return true;
            }
        }
        goto _err;
    }

    _err:
    snprintf(buffer,sizeof(buffer),
        "Backend Select Family Fail, Opcode=%u Dst=%d,Src=%d", mir_node->opcode,mir_node->dst.type,mir_node->src.type);
    FUS_PUSH_ERR(trace,FUSION_ERRO,buffer);
    return false;
}

/*
 * Internal Once Processor
*/
static FusStatusFlag_t X86_ProcessOnceHidr(FusBackendApi_t* FUS, FusBackendGenerateDataBlock_t* block, const FusHidrNode_t* element)
{
    FusTraceTree trace = NULL;
    if (unlikely(!block || !element)) return FUSION_ERRO;
    FUSB_GET_TRACE_FUSION(FUS, &trace);

    x86Instruction_t out_instr = {0};
    X86_MountPrefixHidr(element, &out_instr);
    X86BackendContext backend_ctx = {
        .encoder = &out_instr,
        .hidr    = element,
        .block   = block,
        .Api = FUS
    };

    X86_MountRex(element, &out_instr);
    if (unlikely(!X86_SelectFamily(FUS, &backend_ctx))) return FUSION_ERRO;
    if (unlikely(!X86_MountCodeBytes(&out_instr,
            &block->slab_offset,
            block->buffer_slab,
            block->slab_size))) {
        FUS_PUSH_ERR(trace,FUSION_ERRO,"Backend Process Hidr Step-Fail");
        return FUSION_ERRO;
    }

    FUS_PUSH_ERR(trace,FUSION_OK,"Backend Process Hidr Step-Success");
    return FUSION_OK;
}

#define X86_DEFAULT_ARENA_BLOCK (1*1024)
#define X86_MAX_INSTR_BYTES 15
#define X86_MIN_INSTR_BYTES 6

static inline size_t X86DraticCase(const FusHidrNode_t* node)
{
    if (node->op_size == HIDR_OP_SIZE_64) return X86_MAX_INSTR_BYTES;
    return X86_MIN_INSTR_BYTES;
}

static void DestroyLifetimeBlock(const void* data)
{
    FusBackendGenerateDataBlock_t* block = (FusBackendGenerateDataBlock_t*)data; // EXPLICIT CAST
    FUSB_DESTROY_BLOCK(block->api,block);
}

static FusBackendTransferLifetime_t* X86_BackendMountHidrArry(FusBackendApi_t* FUS, const FusHidrNode_t* hidr, const size_t count)
{
    FusTraceTree trace = NULL;
    if (unlikely(!hidr || count == 0)) return NULL;
    FUSB_GET_TRACE_FUSION(FUS,&trace);

    size_t size_buffer = 0;
    for (size_t i = 0; i < count; i++) size_buffer += X86DraticCase(&hidr[i]);

    FusBackendGenerateDataBlock_t* block = FUSB_CREATE_BLOCK(FUS, 23, size_buffer);
    if (unlikely(!block)) {
        FUS_PUSH_ERR(trace,FUSION_OK,"Backend Create BlockCompiler, Fail");
        return NULL;
    }
    FusBackendTransferLifetime_t* transfer = FUSB_CREATE_TRASNFER(FUS, block, DestroyLifetimeBlock);
    if (unlikely(!transfer)) {
        DestroyLifetimeBlock(block);

        FUS_PUSH_ERR(trace,FUSION_ERRO,"Backend Create TransferLifetime Block, Fail");
        return NULL;
    }

    for (size_t i = 0; i < count; i++) {
        if (count > 32 && i + 8 < count) __builtin_prefetch(&hidr[i + 8], 0, 2);
        FusStatusFlag_t flag = X86_ProcessOnceHidr(FUS, block, &hidr[i]);
        if (flag != FUSION_OK) {
            block->flag = flag;

            FUS_PUSH_ERR(trace,FUSION_OK,"Backend Generation Boundary Reached");
            return transfer;
        }
    }

    block->flag = FUSION_OK;
    FUS_PUSH_ERR(trace,FUSION_OK,"Backend Full Generation ByteCode Step Completed");
    return transfer;
}

static inline void X86_WriteInt32(uint8_t* base, size_t offset, int32_t value)
{
    memcpy(base + offset, &value, sizeof(int32_t));
}
static FusStatusFlag_t X86_LinkerHelper(FusBackendApi_t* FUS, FusBackendRelocationOpaqueType_t opaque_type, FusBackendRelocContext_t* realoc)
{
    FusTraceTree trace = NULL;
    if (unlikely(!realoc)) return FUSION_ERRO;
    FUSB_GET_TRACE_FUSION(FUS,&trace);
    X86ReallocTypes_t type = (X86ReallocTypes_t)opaque_type; 

    switch (type) {
        // 2GB
        case X86_REL32: {
            int64_t delta = (int64_t)realoc->sym_addr - (int64_t)(realoc->patch_addr + 4);
            if (delta > INT32_MAX || delta < INT32_MIN) return FUSION_ERRO;
            X86_WriteInt32(realoc->buffer, realoc->offset, (int32_t)delta);

            FUS_PUSH_ERR(trace,FUSION_OK,"Backend, Relocation REL32 Resolver");
            return FUSION_OK;
        }
        // ABS DIRETO
        case X86_ABS64: {
            uint8_t* local = realoc->buffer + realoc->offset;
            *(uint64_t*)local = (uint64_t)realoc->sym_addr;

            FUS_PUSH_ERR(trace,FUSION_OK,"Backend, Relocation ABS64 Resolver");
            return FUSION_OK;
        }
        // CASSOS INVALIDO
        default: {
            FUS_PUSH_ERR(trace,FUSION_ERRO,"Backend, Relocation Unknow Type");
            return FUSION_ERRO;
        }
    }
}

//     INTERFACE DEFINE     //
#include <Internal/Backend/Fus_StaticBackend.h>

static FusBackendInterface_t interface = {
    .FUSI_BackendMountHidrArray = X86_BackendMountHidrArry,
    .FUSI_BackendLinkerRelocation = X86_LinkerHelper,
};
FusBackendInterface_t* X86_BackendDefine()
{
    return &interface;
}
REGISTER_BACKEND(X86_Backend,X86_BackendDefine);