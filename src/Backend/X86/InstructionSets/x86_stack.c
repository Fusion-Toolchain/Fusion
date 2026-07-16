#include "../x86_helpers.h"
#include "x86_instructions.h"

bool X86_CaseMountPushReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG) return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    mount_instr->opcode.opcode[0] = 0x50 + X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->opcode.opcode_size = 1;

    return true;
}
bool X86_CaseMountPushImm(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_IMM) return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    mount_instr->opcode.opcode[0] = 0x6A;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->imm.value = mir_node->dst.data.imm.imm;
    mount_instr->imm.size = X86_CalMirImmSize(mir_node->dst.data.imm.size);
    mount_instr->has_imm = true;

    return true;
}

bool X86_CaseMountPopReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG) return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    mount_instr->opcode.opcode[0] = 0x58 + X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->opcode.opcode_size = 1;

    return true;
}
bool X86_CaseMountPopImm(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_IMM) return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;
    
    // ADD rsp, imm8/imm32
    // REX.W + 83 /0 ib → ADD r/m64, imm8
    // REX.W + 81 /0 id → ADD r/m64, imm32
    mount_instr->rex.w = 1;
    mount_instr->has_rex = true;
    
    mount_instr->opcode.opcode[0] = 0x83;  // ADD r/m64, imm8 (mais comum)
    mount_instr->opcode.opcode_size = 1;
    
    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT;
    mount_instr->modrm.reg = 0;  // /0 = ADD opcode
    mount_instr->modrm.rm = X86_REG_RSP;
    mount_instr->has_modrm = true;
    
    mount_instr->imm.value = mir_node->dst.data.imm.imm;
    mount_instr->imm.size = 1;  // imm8
    mount_instr->has_imm = true;
    
    return true;
}