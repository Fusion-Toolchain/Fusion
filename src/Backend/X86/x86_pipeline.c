#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "x86_types.h"
#include "x86_functions.h"

struct CopyPartMemory {
    size_t* offset;
    unsigned char* dst;
    size_t dst_size;
    unsigned char* src;
    size_t src_size;
};
typedef bool (*EncodeStep)(x86Instruction_t*,struct CopyPartMemory*);
typedef struct {
    EncodeStep steps[8];
    size_t count;
} EncodePipeline;

static bool CopyOffsetData(struct CopyPartMemory* step)
{
    if (!step->offset || !step->src || !step->dst) return false;
    if ((*step->offset) + step->src_size > step->dst_size) return false;

    memcpy(step->dst + *step->offset, step->src, step->src_size);
    *step->offset += step->src_size;

    return true;
}
static inline bool CopyOffsetDataU8(struct CopyPartMemory* step, uint8_t value)
{
    if (!step->offset || !step->dst) return false;
    if ((*step->offset) + 1 > step->dst_size) return false;

    step->dst[(*step->offset)++] = value;
    return true;
}

static bool x86Bytes_MountOpcode(x86Instruction_t* instr, struct CopyPartMemory* mounter)
{
    mounter->src = instr->opcode.opcode;
    mounter->src_size = instr->opcode.opcode_size;
    if (!CopyOffsetData(mounter)) return false;
    return true;
}

static inline uint8_t MountModRM(x86ModRm_t* modrm)
{
    if (
        modrm->mod > MODRM_MOD_MAX_VALUE ||
        modrm->reg > MODRM_REG_MAX_VALUE ||
        modrm->rm > MODRM_RM_MAX_VALUE
    ){
        return 0;
    }

    return (modrm->mod << 6) | (modrm->reg << 3) | modrm->rm;
}
static bool x86Bytes_MountModRM(x86Instruction_t* instr, struct CopyPartMemory* mounter)
{
    if (!instr->has_modrm) return true;

    uint8_t modrm = MountModRM(&instr->modrm);
    if (!CopyOffsetDataU8(mounter, modrm)) return false;
    return true;
}
static bool x86Bytes_MountImm(x86Instruction_t* instr, struct CopyPartMemory* mounter)
{
    if (!instr->has_imm) return true;

    mounter->src = (unsigned char*)&instr->imm.value;
    mounter->src_size = instr->imm.size;
    if (!CopyOffsetData(mounter)) return false;
    return true;
}
static bool x86Bytes_MountPrefix(x86Instruction_t* instr, struct CopyPartMemory* mounter)
{
    if (!instr->has_prefix) return true;

    mounter->src = instr->prefix.prefix;
    mounter->src_size = instr->prefix.prefix_size;
    if (!CopyOffsetData(mounter)) return false;
    return true;
}

static inline uint8_t MountSIB(x86Sib_t* sib)
{
    return (sib->scale << 6) | (sib->index << 3) | sib->base;
}
static bool x86Bytes_MountSIB(x86Instruction_t* instr, struct CopyPartMemory* mounter)
{
    if (!instr->has_sib) return true;

    uint8_t sib = MountSIB(&instr->sib);
    return CopyOffsetDataU8(mounter, sib);
}

static bool x86Bytes_MountDisp(x86Instruction_t* instr, struct CopyPartMemory* mounter)
{
    if (!instr->has_disp) return true;

    mounter->src = (unsigned char*)&instr->disp.value;
    mounter->src_size = instr->disp.size;

    return CopyOffsetData(mounter);
}

static bool x86Bytes_MountRex(x86Instruction_t* instr, struct CopyPartMemory* mounter)
{
    if (!instr->has_rex) return true;

    x86Rex_t* rex = &instr->rex;
    uint8_t rex_bytes = 0x40 |
        (rex->w << 3) |
        (rex->r << 2) |
        (rex->x << 1) |
        (rex->b << 0);
    
    mounter->src = &rex_bytes;
    mounter->src_size = 1;

    return CopyOffsetData(mounter);
}

static EncodePipeline pipeline_funcs = {
    .steps = {
        [0]=x86Bytes_MountPrefix,
        [1]=x86Bytes_MountRex,
        [2]=x86Bytes_MountOpcode,
        [3]=x86Bytes_MountModRM,
        [4]=x86Bytes_MountSIB,
        [5]=x86Bytes_MountDisp,
        [6]=x86Bytes_MountImm,
    },
    .count = 7 // 0 .. 6
};

bool X86_MountCodeBytes(x86Instruction_t* instr, size_t* offset, uint8_t* buffer, size_t buffer_size)
{
    if (!instr || !buffer || !offset) return false;
    struct CopyPartMemory mounter_copy = {
        .dst = buffer,
        .dst_size = buffer_size,
        .offset = offset,
    };

    for (size_t i = 0; i < pipeline_funcs.count; i++) {
        if (!pipeline_funcs.steps[i](instr,&mounter_copy)) return false;
    }
    return true;
}