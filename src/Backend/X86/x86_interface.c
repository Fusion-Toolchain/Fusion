/*
 * CR: Implementa sistema Validação de HIDR para arquitetura do Backend
 * PS: Fazer algo modular para interface do backend, para todo backend seguir a definição de regras.
 * CR: Corriger geração de codigo para organizar melhor, arquivos diferente e pipeline.
 * PR: Nova Interface melhorada, para backend, algo como Backend API.
*/

#include <Fusion/IRTypes/HidrType.h>
#include <Fusion/FusionTypes.h>
#include <Internal/Fus_Backend.h>
#include <Internal/Memory/Fus_Arena.h>
#include <Fusion/Fusion.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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
#include <string.h>
static FusStatusFlag_t X86_ProcessOnceHidr(FusBackendGenereteDataBlock_t* block,FusBufferContext_t* buffer, const FusHidrNode_t* element)
{
    if (!buffer || !element) return FUSION_ERRO;

    x86Instruction_t out_instr = {0};
    X86_MountPrefixHidr(element,&out_instr);

    X86BackendContext backend_ctx = {
        .encoder = &out_instr,
        .hidr = element,
        .block = block
    };

    if (!X86_SelectFamily(&backend_ctx)) return FUSION_ERRO;
    X86_MountRex(element,&out_instr);

    if (!X86_MountCodeBytes(&out_instr, &buffer->offset, buffer->buffer, buffer->buffer_size)) {
        return FUSION_ERRO;
    }

    return FUSION_OK;
}

#define X86_DEFAULT_ARENA_BLOCK (1*1024)
#define X86_MAX_INSTR_BYTES 15
static inline FusBackendGenereteDataBlock_t* X86_MountBlockReturned(size_t needs, size_t instr_count)
{
    FusMemoryArena_t* arena = FUSI_CreateArena(X86_DEFAULT_ARENA_BLOCK);
    if (!arena) return NULL;

    FusBackendGenereteDataBlock_t* block = FUS->FusAlloc(FUS,sizeof(FusBackendGenereteDataBlock_t));
    if (!block) {
        FUSI_DestroyArena(arena);
        return NULL;
    }

    FusBackendReallocNeed_t* block_needs = FUS->FusAlloc(FUS,sizeof(FusBackendReallocNeed_t) * needs);
    if (!block_needs) {
        FUS->FusFree(FUS,block);
        FUSI_DestroyArena(arena);
        return NULL;
    }
    size_t size_buffer = X86_MAX_INSTR_BYTES*instr_count;
    FusBufferContext_t* buffer = FUS_CreateBufferCode(size_buffer);
    if (!buffer) {
        FUS->FusFree(FUS,block);
        FUS->FusFree(FUS,block_needs);
        FUSI_DestroyArena(arena);
        return NULL;
    }

    block->buffer = buffer;
    block->arena = arena;
    block->realoc = block_needs;
    block->realoc_count = 0;
    block->realoc_capacity = needs;
    block->flag = FUSION_OK;

    return block;
}
static void FreeBlock(const void* data)
{
    FusBackendGenereteDataBlock_t* block = (FusBackendGenereteDataBlock_t*)data;

    FUSI_DestroyArena(block->arena);
    FUS_DestroyBufferCode(block->buffer);
    FUS->FusFree(FUS,block->realoc);
    FUS->FusFree(FUS,block);
}

static FusBackendTrasferLifeTime_t* X86_BackendMountHidr(const FusHidrNode_t* mir_node)
{
    if (!mir_node) return NULL;

    FusBackendGenereteDataBlock_t* block = X86_MountBlockReturned(23,1);
    if (!block) return NULL;
    FusBackendTrasferLifeTime_t* block_trasfer = FUS->FusAlloc(FUS,sizeof(FusBackendTrasferLifeTime_t));
    if (!block_trasfer) {
        FreeBlock(block);
        return NULL;
    }
    block_trasfer->data = block;
    block_trasfer->free = FreeBlock;

    FusStatusFlag_t flag = X86_ProcessOnceHidr(block,block->buffer, mir_node);
    if (flag != FUSION_OK) {
        block->flag = flag;
        return block_trasfer;
    }

    return block_trasfer;
}
static FusBackendTrasferLifeTime_t* X86_BackendMountHidrArry(const FusHidrNode_t* hidr, const size_t count)
{
    if (!hidr || count == 0) return NULL;

    FusBackendGenereteDataBlock_t* block = X86_MountBlockReturned(23,count);
    if (!block) return NULL;
    FusBackendTrasferLifeTime_t* block_trasfer = FUS->FusAlloc(FUS,sizeof(FusBackendTrasferLifeTime_t));
    if (!block_trasfer) {
        FreeBlock(block);
        return NULL;
    }
    block_trasfer->data = block;
    block_trasfer->free = FreeBlock;

    for (size_t i = 0; i < count; i++) {
        FusStatusFlag_t flag = X86_ProcessOnceHidr(block,block->buffer,&hidr[i]);
        if (flag != FUSION_OK) {
            block->flag = flag;
            return block_trasfer;
        }
    }

    return block_trasfer;
}

static FusStatusFlag_t X86_LinkerHelper(FusBackendRealocOpaqueType_t opaque_type,FusBackendRelocContext_t* realoc)
{
    if (!realoc) return FUSION_ERRO;
    X86ReallocTypes_t type = (X86ReallocTypes_t)opaque_type;

    switch (type) {
        case X86_REL32: {
            uint8_t* local = (uint8_t*)(realoc->buffer->buffer + realoc->offset);
            int64_t delta = (int64_t)realoc->sym_addr - (int64_t)(realoc->patch_addr + 4);
            if (delta > INT32_MAX || delta < INT32_MIN) return FUSION_ERRO;

            *(int32_t*)local = (int32_t)delta;
            return FUSION_OK;
        }
        case X86_ABS64: {
            uint8_t* local = (uint8_t*)(realoc->buffer->buffer + realoc->offset);
            *(uint64_t*)local = (uint64_t)realoc->sym_addr;
            return FUSION_OK;
        }
        default: {
            return FUSION_ERRO;
        }
    }

    return FUSION_OK;
}

//     INTERFACE DEFINE
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