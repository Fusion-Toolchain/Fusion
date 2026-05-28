#ifndef FUSION_HIDR_HELPER_H
#define FUSION_HIDR_HELPER_H

#include <Fusion/IRTypes/HidrType.h>

#include <stdbool.h>
#include <stdint.h>

// HELPERS

static inline FusHidrOperandType_t FusMirGetSrcType(const FusHidrNode_t* n) { return n->src.type; }
static inline FusHidrOperandType_t FusMirGetDstType(const FusHidrNode_t* n) { return n->dst.type; }

static inline FusHidrVirtualReg_t FusMirGetSrcReg(const FusHidrNode_t* n) { return n->src.data.reg; }
static inline FusHidrVirtualReg_t FusMirGetDstReg(const FusHidrNode_t* n) { return n->dst.data.reg; }

static inline int64_t  FusMirGetSrcImm(const FusHidrNode_t* n) { return n->src.data.imm.imm; }
static inline int64_t  FusMirGetDstImm(const FusHidrNode_t* n) { return n->dst.data.imm.imm; }

static inline FusHidrImmSize_t FusMirGetSrcImmSize(const FusHidrNode_t* n) { return n->src.data.imm.size; }
static inline FusHidrImmSize_t FusMirGetDstImmSize(const FusHidrNode_t* n) { return n->dst.data.imm.size; }

static inline FusHidrVirtualReg_t FusMirGetMemRefBase(const FusHidrNode_t* n, bool src)
{
    return src ? n->src.data.memory_ref.base : n->dst.data.memory_ref.base;
}

static inline int16_t FusMirGetMemRefOffset(const FusHidrNode_t* n, bool src)
{
    return src ? (int16_t)n->src.data.memory_ref.offset : (int16_t)n->dst.data.memory_ref.offset;
}
#endif