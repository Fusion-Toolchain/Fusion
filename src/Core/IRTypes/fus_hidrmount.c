#include "Fusion/IRTypes/HidrType.h"
#include <Internal/IRTypes/Fus_CodeBuffer.h>
#include <Internal/Fus_Instance.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>

// TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

FusStatusFlag_t FUS_CreateCodeMount(FusInstance* ctx, FusCodeMount_t* out)
{
    if (unlikely(!ctx || !out)) return FUSION_ERRO;
    struct FusInstance_T* instance_real = *ctx;

    *out = NULL;

    struct FusCodeMount_T* code_ctx = FUSIH_ALLOC(instance_real->allocation, sizeof(struct FusCodeMount_T));
    if (unlikely(!code_ctx)) return FUSION_ERRO;

    code_ctx->code_capacity = 10;

    size_t init_size = code_ctx->code_capacity * sizeof(FusHidrNode_t);
    FusHidrNode_t* code_hidr = FUSIH_ALLOC(instance_real->allocation,init_size);
    if (unlikely(!code_hidr)) {
        FUSIH_FREE(instance_real->allocation,code_ctx);
        return FUSION_ERRO;
    }

    code_ctx->code_arry = code_hidr;
    code_ctx->code_count = 0;

    code_ctx->ref_ctx = instance_real;

    *out = code_ctx;
    return FUSION_OK;
}

static inline bool ExpansiveCodeBuffer(struct FusCodeMount_T* mount)
{
    if (unlikely(!mount)) return false;
    struct FusInstance_T* ctx = mount->ref_ctx;

    if (mount->code_count + 1 > mount->code_capacity) {
        size_t new_capacity = mount->code_capacity * 2;
        size_t new_size = new_capacity * sizeof(FusHidrNode_t);

        FusHidrNode_t* new_arry = FUSIH_REALLOC(ctx->allocation,mount->code_arry,new_size);
        if (unlikely(!new_arry)) return false;

        mount->code_arry = new_arry;
        mount->code_capacity = new_capacity;
    }

    return true;
}
static inline FusStatusFlag_t InsertHidrArryHelper(struct FusCodeMount_T* mount, FusHidrNode_t node)
{
    if (unlikely(!mount)) return FUSION_ERRO;
    if (unlikely(!ExpansiveCodeBuffer(mount))) return FUSION_ERRO;

    mount->code_arry[mount->code_count] = node;
    mount->code_count++;

    return FUSION_OK;
}

FusStatusFlag_t FUS_NewMovSym(FusCodeMount_t mount, const char* sym_name, int dst_reg)
{
    if (unlikely(!mount)) return FUSION_ERRO;

    FusHidrNode_t node = {
        .opcode = HIDR_INSTR_MOV,
        .op_size = HIDR_OP_SIZE_64,
        .mode = HIDR_MODE64,
        .src = { .type = HIDR_OPERAND_TYPE_SYM, .data.sym.name = sym_name },
        .dst = { .type = HIDR_OPERAND_TYPE_REG, .data.reg = dst_reg }
    };

    return InsertHidrArryHelper(mount,node);
}
FusStatusFlag_t FUS_NewCallReg(FusCodeMount_t mount, int src_reg)
{
    if (unlikely(!mount)) return FUSION_ERRO;

    FusHidrNode_t node = {
        .opcode = HIDR_INSTR_CALL,
        .op_size = HIDR_OP_SIZE_32,
        .src = { .type = HIDR_OPERAND_TYPE_REG, .data.reg = src_reg }
    };

    return InsertHidrArryHelper(mount,node);
}
FusStatusFlag_t FUS_NewRet(FusCodeMount_t mount)
{
    if (unlikely(!mount)) return FUSION_ERRO;

    FusHidrNode_t node = {
        .opcode = HIDR_INSTR_RET
    };

    return InsertHidrArryHelper(mount,node);
}
FusStatusFlag_t FUS_NewAddrMem(FusCodeMount_t mount,int base_reg, int offset, int dst_reg)
{
    if (unlikely(!mount)) return FUSION_ERRO;

    FusHidrNode_t node = {
        .opcode = HIDR_INSTR_ADDR,
        .op_size = HIDR_OP_SIZE_64,
        .mode = HIDR_MODE64,
        .src = {
            .type = HIDR_OPERAND_TYPE_MEM_REF,
            .data.memory_ref = { .base = base_reg, .offset = offset }
        },
        .dst = { .type = HIDR_OPERAND_TYPE_REG, .data.reg = dst_reg }
    };

    return InsertHidrArryHelper(mount,node);
}
FusStatusFlag_t FUS_NewMov(FusCodeMount_t mount, FusHidrOperand_t dst, FusHidrOperand_t src)
{
    if (unlikely(!mount)) return FUSION_ERRO;

    FusHidrNode_t node = {
        .opcode = HIDR_INSTR_MOV,
        .op_size = HIDR_OP_SIZE_64,
        .mode = HIDR_MODE64,
        .dst = dst,
        .src = src
    };

    return InsertHidrArryHelper(mount, node);
}


// FUNÇOES DE REMENDAGEM, temporario!
size_t FUS_GetCountCode(FusCodeMount_t mount)
{
    if (unlikely(!mount)) return 0;
    return mount->code_count;
}
FusHidrNode_t* FUS_GetArryCode(FusCodeMount_t mount)
{
    if (unlikely(!mount)) return NULL;
    return mount->code_arry;
}

void FUS_DestroyCodeMount(FusInstance* ctx, FusCodeMount_t code)
{
    if (unlikely(!ctx || !code)) return;
    struct FusInstance_T* instance_real = *ctx;

    FUSIH_FREE(instance_real->allocation,code->code_arry);
    FUSIH_FREE(instance_real->allocation,code);

    code = NULL;
}