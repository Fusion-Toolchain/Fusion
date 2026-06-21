/*
 * CR: Implementa sistema Validação de HIDR para arquitetura do Backend
 * PS: Fazer algo modular para interface do backend, para todo backend seguir a definição de regras.
 * PR: Nova Interface melhorada, para backend, algo como Backend API.
*/

#include <Fusion/IRTypes/HidrType.h>
#include <Fusion/FusionTypes.h>
#include <Internal/Fus_Backend.h>
#include <Internal/Memory/Fus_Arena.h>
#include <Fusion/Fusion.h>

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

FusBackendApi_t* FUS = NULL;

#include "InstructionSets/x86_instructions.h" // IMPORT CASES
#include "x86_FamilyDefine.inc" // DEFINE CASES

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

    if (mir_node->mode != HIDR_MODE64) return;

    // W: operação 64-bit
    if (mir_node->op_size == HIDR_OP_SIZE_64 &&
    !(mir_node->src.type == HIDR_OPERAND_TYPE_IMM && 
    mir_node->src.data.imm.size == HIDR_IMM8)) {
        instr->rex.w = 1;
        instr->has_rex = true;
    }

    // R: extensão do campo reg (dst normalmente)
    if (mir_node->dst.type == HIDR_OPERAND_TYPE_REG &&
        mir_node->dst.data.reg >= 8) {
        instr->rex.r = 1;
        instr->has_rex = true;
    }

    // B: extensão do campo rm (src normalmente)
    if (mir_node->src.type == HIDR_OPERAND_TYPE_REG &&
        mir_node->src.data.reg >= 8) {
        instr->rex.b = 1;
        instr->has_rex = true;
    }

    // memória (base register)
    if (mir_node->src.type == HIDR_OPERAND_TYPE_MEM_REF) {
        if (mir_node->src.data.memory_ref.base >= 8) {
            instr->rex.b = 1;
            instr->has_rex = true;
        }
    }
}

static inline bool X86_SelectFamily(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;

    for (size_t i = 0; i < (sizeof(familys) / sizeof(familys[0])); i++) {
        if (familys[i].opcode != mir_node->opcode) continue;
        for (size_t j = 0; j < familys[i].rule_count; j++) {
            X86FamilyRule_t* rules = &familys[i].rules[j];
            if (rules->dst_type != mir_node->dst.type) continue;
            if (rules->src_type != mir_node->src.type) continue;

            if (rules->builder(backend_ctx)) return true;
        }
        return false;
    }
    return false;
}

/*
 * Internal Once Processor
*/
static FusStatusFlag_t X86_ProcessOnceHidr(FusBackendGenereteDataBlock_t* block, const FusHidrNode_t* element)
{
    if (!block || !element) return FUSION_ERRO;

    x86Instruction_t out_instr = {0};
    X86_MountPrefixHidr(element, &out_instr);

    X86BackendContext backend_ctx = {
        .encoder = &out_instr,
        .hidr    = element,
        .block   = block
    };

    if (!X86_SelectFamily(&backend_ctx)) return FUSION_ERRO;
    X86_MountRex(element, &out_instr);

    if (!X86_MountCodeBytes(&out_instr,
            &block->slab_offset,
            block->buffer_slab,
            block->slab_size)) {
        return FUSION_ERRO;
    }

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
    FusBackendGenereteDataBlock_t* block = (FusBackendGenereteDataBlock_t*)data; // EXPLICIT CAST

    FUSB_DESTROY_BLOCK(block->api,block);
}

static FusBackendTrasferLifeTime_t* X86_BackendMountHidr(const FusHidrNode_t* mir_node)
{
    if (!mir_node) return NULL;

    size_t size_buffer = X86DraticCase(mir_node);
    FusBackendGenereteDataBlock_t* block = FUSB_CREATE_BLOCK(FUS, 23, size_buffer);
    if (!block) return NULL;

    FusBackendTrasferLifeTime_t* transfer = FUSB_CREATE_TRASNFER(FUS, block, DestroyLifetimeBlock);
    if (!transfer) {
        DestroyLifetimeBlock(block);
        return NULL;
    }

    FusStatusFlag_t flag = X86_ProcessOnceHidr(block, mir_node);
    if (flag != FUSION_OK) {
        block->flag = flag;
    }

    return transfer;
}

static FusBackendTrasferLifeTime_t* X86_BackendMountHidrArry(const FusHidrNode_t* hidr, const size_t count)
{
    if (!hidr || count == 0) return NULL;

    size_t size_buffer = 0;
    for (size_t i = 0; i < count; i++)
        size_buffer += X86DraticCase(&hidr[i]);

    FusBackendGenereteDataBlock_t* block = FUSB_CREATE_BLOCK(FUS, 23, size_buffer);
    if (!block) return NULL;

    FusBackendTrasferLifeTime_t* transfer = FUSB_CREATE_TRASNFER(FUS, block, DestroyLifetimeBlock);
    if (!transfer) {
        DestroyLifetimeBlock(block);
        return NULL;
    }

    for (size_t i = 0; i < count; i++) {
        FusStatusFlag_t flag = X86_ProcessOnceHidr(block, &hidr[i]);
        if (flag != FUSION_OK) {
            block->flag = flag;
            return transfer;
        }
    }

    return transfer;
}

static inline void X86_WriteInt32(uint8_t* base, size_t offset, int32_t value)
{
    memcpy(base + offset, &value, sizeof(int32_t));
}

static FusStatusFlag_t X86_LinkerHelper(FusBackendRealocOpaqueType_t opaque_type, FusBackendRelocContext_t* realoc)
{
    if (!realoc) return FUSION_ERRO;
    X86ReallocTypes_t type = (X86ReallocTypes_t)opaque_type;

    switch (type) {
        // 2GB
        case X86_REL32: {
            int64_t delta = (int64_t)realoc->sym_addr - (int64_t)(realoc->patch_addr + 4);
            if (delta > INT32_MAX || delta < INT32_MIN) return FUSION_ERRO;
            X86_WriteInt32(realoc->buffer, realoc->offset, (int32_t)delta);
            return FUSION_OK;
        }
        // ABS DIRETO
        case X86_ABS64: {
            uint8_t* local = realoc->buffer + realoc->offset;
            *(uint64_t*)local = (uint64_t)realoc->sym_addr;
            return FUSION_OK;
        }
        // CASSO INVALIDO
        default:
            return FUSION_ERRO;
    }
}

//     INTERFACE DEFINE     //
#include <Internal/Fus_StaticBackend.h>

static FusBackendInterface_t interface = {
    .FUSI_BackendMountHidr = X86_BackendMountHidr,
    .FUSI_BackendMountHidrArry = X86_BackendMountHidrArry,
    .FUSI_BackendLinkerRealloc = X86_LinkerHelper,
};
FusBackendInterface_t* X86_BackendDefine(FusBackendApi_t* api)
{
    FUS = api;
    return &interface;
}
REGISTER_BACKEND(X86_Backend,X86_BackendDefine);