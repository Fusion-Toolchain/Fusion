#include "../x86_helpers.h"
#include "x86_instructions.h"

bool X86_CaseMountAddRegReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    mount_instr->opcode.opcode[0] = 0x01;  // ADD r/m, r
    mount_instr->opcode.opcode_size = 1;

    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT;
    mount_instr->modrm.reg = X86_MapVirtualReg(mir_node->src.data.reg);
    mount_instr->modrm.rm  = X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->has_modrm = true;

    return true;
}

bool X86_CaseMountAddImmReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    mount_instr->opcode.opcode[0] = 0x81;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT;
    mount_instr->modrm.reg = 0;  // /0 = ADD
    mount_instr->modrm.rm  = X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->has_modrm = true;

    size_t imm_size = X86_CalMirImmSize(mir_node->src.data.imm.size);
    if (!imm_size) return false;

    mount_instr->imm.value = mir_node->src.data.imm.imm;
    mount_instr->imm.size  = imm_size;
    mount_instr->has_imm   = true;

    return true;
}