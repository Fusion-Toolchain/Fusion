#include "../x86_helpers.h"
#include "x86_instructions.h"

#include <stdbool.h>

bool X86_CaseMountCallReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder; 

    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG) return false;

    size_t src_reg = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    if (src_reg == (size_t)-1) return false;

    /* Opcode: FF */
    mount_instr->opcode.opcode[0] = 0xFF;
    mount_instr->opcode.opcode_size = 1;

    /*
     * ModRM: mod=11 (reg direct), reg=2 (/2 = CALL), rm=src
     * Em 64-bit mode FF /2 já opera em 64-bit — REX.W não
     * é necessário (e na verdade é ignorado pelo processador).
     */
    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT; /* 0b11 */
    mount_instr->modrm.reg = 2;                     /* /2   */
    mount_instr->modrm.rm  = (uint8_t)(src_reg & 0x7);
    mount_instr->has_modrm = true;

    mount_instr->rex.w   = 0;
    mount_instr->rex.b   = (src_reg > 7) ? 1 : 0;
    mount_instr->has_rex = (src_reg > 7);

    return true;
}

bool X86_CaseMountCallRel32(X86BackendContext* backend_ctx)
{
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    mount_instr->opcode.opcode[0] = 0xE8;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->imm.value = 0;
    mount_instr->imm.size  = 4; /* rel32 — sempre 4 bytes */
    mount_instr->has_imm   = true;

    return true;
}