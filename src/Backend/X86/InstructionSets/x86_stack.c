#include "../x86_helpers.h"
#include "x86_instructions.h"

bool X86_CaseMountPushReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG)  return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    size_t mapped = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    if (mapped == (size_t)-1) return false;

    mount_instr->opcode.opcode[0] = 0x50 + (mapped & 0x7);
    mount_instr->opcode.opcode_size = 1;

    if (mapped >= 8) {
        mount_instr->rex.b   = 1;
        mount_instr->has_rex = true;
    }

    return true;
}

bool X86_CaseMountPushImm(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_IMM)  return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    size_t imm_size = X86_CalMirImmSize(mir_node->dst.data.imm.size);
    if (!imm_size) return false;

    mount_instr->opcode.opcode[0] = 0x6A;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->imm.value = mir_node->dst.data.imm.imm;
    mount_instr->imm.size  = imm_size;
    mount_instr->has_imm   = true;

    return true;
}

bool X86_CaseMountPopReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG)  return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    size_t mapped = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    if (mapped == (size_t)-1) return false;

    mount_instr->opcode.opcode[0] = 0x58 + (mapped & 0x7);
    mount_instr->opcode.opcode_size = 1;

    if (mapped >= 8) {
        mount_instr->rex.b   = 1;
        mount_instr->has_rex = true;
    }

    return true;
}

bool X86_CaseMountPopImm(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_IMM)  return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    mount_instr->rex.w     = 1;
    mount_instr->has_rex   = true;

    mount_instr->opcode.opcode[0] = 0x83;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT;
    mount_instr->modrm.reg = 0;
    mount_instr->modrm.rm  = X86_REG_RSP;
    mount_instr->has_modrm = true;

    mount_instr->imm.value = mir_node->dst.data.imm.imm;
    mount_instr->imm.size  = 1;
    mount_instr->has_imm   = true;

    return true;
}