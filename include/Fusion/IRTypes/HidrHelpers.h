#ifndef FUSION_HIDR_HELPER_H
#define FUSION_HIDR_HELPER_H

#include "HidrType.h"
#include <stdbool.h>

static inline FusHidrOperandType_t FusMirGetSrcType(FusHidrNode_t* n) { return n->src.type; }
static inline FusHidrOperandType_t FusMirGetDstType(FusHidrNode_t* n) { return n->dst.type; }

static inline FusHidrVirtualReg_t FusMirGetSrcReg(FusHidrNode_t* n) { return n->src.data.reg; }
static inline FusHidrVirtualReg_t FusMirGetDstReg(FusHidrNode_t* n) { return n->dst.data.reg; }

static inline int64_t  FusMirGetSrcImm(FusHidrNode_t* n) { return n->src.data.imm.imm; }
static inline int64_t  FusMirGetDstImm(FusHidrNode_t* n) { return n->dst.data.imm.imm; }

static inline FusHidrImmSize_t FusMirGetSrcImmSize(FusHidrNode_t* n) { return n->src.data.imm.size; }
static inline FusHidrImmSize_t FusMirGetDstImmSize(FusHidrNode_t* n) { return n->dst.data.imm.size; }

static inline FusHidrVirtualReg_t FusMirGetMemRefBase(FusHidrNode_t* n, bool src)
{
    return src ? n->src.data.memory_ref.base : n->dst.data.memory_ref.base;
}
static inline uint16_t FusMirGetMemRefOffset(FusHidrNode_t* n, bool src)
{
    return src ? n->src.data.memory_ref.offset : n->dst.data.memory_ref.offset;
}
static inline FusHidrRegClass_t FusHidrGetRegClass(FusHidrVirtualReg_t reg)
{
    if (reg < 0)                        return HIDR_VREG_CLASS_VIRTUAL;
    if (reg < HIDR_VREG_SPECIAL_BASE)    return HIDR_VREG_CLASS_GENERAL;
    if (reg < HIDR_VREG_STACK_BASE)      return HIDR_VREG_CLASS_SPECIAL;
    return                                     HIDR_VREG_CLASS_STACK;
}

#endif