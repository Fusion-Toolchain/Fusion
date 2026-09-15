#include "../x86_helpers.h"
#include "x86_instructions.h"

// HELPER
#include <BackendInterface/Backend.h>

#include <stdbool.h>
#include <stddef.h>

bool X86_CaseMountMovImmReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    FusHidrImmSize_t imm_type = mir_node->src.data.imm.size;
    size_t imm_size = X86_CalMirImmSize(imm_type);
    if (!imm_size) return false;

    size_t mapped = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    if (mapped == (size_t)-1) return false;

    mount_instr->opcode.opcode[0] = 0xB8 + (mapped & 0x7);
    mount_instr->opcode.opcode_size = 1;

    mount_instr->has_rex = false;
    if (mir_node->op_size == HIDR_OP_SIZE_64) {
        mount_instr->rex.w = 1;
        mount_instr->has_rex = true;
        if (mapped >= 8) mount_instr->rex.b = 1;
        if (imm_type <= HIDR_IMM32) mount_instr->imm.size = 4;
        else                        mount_instr->imm.size = 8;
    } else {
        if (mapped >= 8) {
            mount_instr->rex.b = 1;
            mount_instr->has_rex = true;
        }
        mount_instr->imm.size = imm_size;
    }

    mount_instr->imm.value = mir_node->src.data.imm.imm;
    mount_instr->has_imm = true;
    return true;
}

bool X86_CaseMountMovRegReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    size_t dst_reg = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    size_t src_reg = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->src.data.reg));
    if (dst_reg == (size_t)-1 || src_reg == (size_t)-1) return false;

    mount_instr->opcode.opcode[0] = 0x89;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT;
    mount_instr->modrm.reg = dst_reg & 0x7;
    mount_instr->modrm.rm  = src_reg & 0x7;
    mount_instr->has_modrm = true;
    return true;
}

bool X86_CaseMountMovMemImm(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* instr = backend_ctx->encoder;

    size_t reg_mem = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.memory_ref.base));
    if (reg_mem == (size_t)-1) return false;

    int32_t offset = mir_node->dst.data.memory_ref.offset;

    instr->opcode.opcode[0] = 0xC7;
    instr->opcode.opcode_size = 1;
    instr->modrm.reg = 0;

    if (offset >= -128 && offset <= 127) {
        instr->disp.size = 1;
        instr->modrm.mod = 1;
    } else {
        instr->disp.size = 4;
        instr->modrm.mod = 2;
    }

    instr->disp.value = offset;
    instr->has_disp = true;

    if (reg_mem == 4) {
        instr->modrm.rm = 4;
        instr->has_sib = true;
        instr->sib.scale = X86_SIB_SCALE_1;
        instr->sib.index = X86_SIB_INDEX_NONE;
        instr->sib.base  = 4;
    } else {
        instr->modrm.rm = reg_mem;
        instr->has_sib = false;
    }

    instr->has_modrm = true;
    instr->imm.value = mir_node->src.data.imm.imm;
    instr->imm.size = 4;
    instr->has_imm = true;
    return true;
}

bool X86_CaseMountMovRegMem(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* instr = backend_ctx->encoder;

    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG)    return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_MEM_REF) return false;

    size_t dst_reg  = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    size_t base_reg = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->src.data.memory_ref.base));
    if (dst_reg == (size_t)-1 || base_reg == (size_t)-1) return false;

    int32_t offset = mir_node->src.data.memory_ref.offset;

    instr->opcode.opcode[0] = 0x8B;
    instr->opcode.opcode_size = 1;

    uint8_t mod;
    if (offset == 0 && (base_reg & 0x7) != 5) mod = 0;
    else if (offset >= -128 && offset <= 127)  mod = 1;
    else                                        mod = 2;

    instr->modrm.mod = mod;
    instr->modrm.reg = dst_reg & 0x7;
    instr->modrm.rm  = base_reg & 0x7;
    instr->has_modrm = true;

    instr->rex.w = 1;
    instr->rex.r = (dst_reg  >= 8);
    instr->rex.b = (base_reg >= 8);
    instr->has_rex = true;

    if (mod == 1) {
        instr->disp.value = offset;
        instr->disp.size  = 1;
        instr->has_disp   = true;
    } else if (mod == 2) {
        instr->disp.value = offset;
        instr->disp.size  = 4;
        instr->has_disp   = true;
    }
    return true;
}

bool X86_CaseMountMovSymReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node    = backend_ctx->hidr;
    x86Instruction_t*    mount_instr = backend_ctx->encoder;

    size_t mapped = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    if (mapped == (size_t)-1) return false;

    mount_instr->opcode.opcode[0]   = 0xB8 + (mapped & 0x7);
    mount_instr->opcode.opcode_size = 1;

    if (mapped >= 8) {
        mount_instr->rex.b   = 1;
        mount_instr->has_rex = true;
    }

    mount_instr->imm.value = 0xFFFFFFFFFFFFFFFF;
    mount_instr->imm.size  = 8;
    mount_instr->has_imm   = true;

    size_t offset = backend_ctx->block->slab_offset + 2;
    FUSB_REGISTRE_REALOCATION(
        backend_ctx->Api, backend_ctx->block,
        mir_node->src.data.sym.name, X86_ABS64, offset
    );
    return true;
}