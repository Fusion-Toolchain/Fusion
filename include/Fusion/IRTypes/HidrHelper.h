#ifndef FUSION_PUBLIC_CODEMOUNT_H
#define FUSION_PUBLIC_CODEMOUNT_H
#include <Fusion/FusionTypes.h>
#include "HidrType.h"

FUS_DEFINE_HANDLE(FusCodeMount);

FusStatusFlag_t FUS_CreateCodeMount(FusInstance* ctx, FusCodeMount* out);
void FUS_DestroyCodeMount(FusInstance* ctx, FusCodeMount code);

FusStatusFlag_t FUS_InsertCodeBlock(struct FusCodeMount_T* mount, FusHidrNode_t node);

static inline FusHidrNode_t FUS_HIDRM(
    FusHidrNodeKind_t   op,
    FusHidrOpcodeSize_t size,
    FusHidrOperand_t    dst,
    FusHidrOperand_t    src)
{
    return (FusHidrNode_t){
        .opcode  = op,
        .op_size = size,
        .dst     = dst,
        .src     = src
    };
}

#define FUS_REG(r)      (FusHidrOperand_t){ .type = HIDR_OPERAND_TYPE_REG,     .data.reg = (r) }
#define FUS_IMM(v, s)   (FusHidrOperand_t){ .type = HIDR_OPERAND_TYPE_IMM,     .data.imm = { .imm = (v), .size = (s) } }
#define FUS_SYM(n)      (FusHidrOperand_t){ .type = HIDR_OPERAND_TYPE_SYM,     .data.sym.name = (n) }
#define FUS_MEM(b, o)   (FusHidrOperand_t){ .type = HIDR_OPERAND_TYPE_MEM_REF, .data.memory_ref = { .base = (b), .offset = (o) } }

#endif