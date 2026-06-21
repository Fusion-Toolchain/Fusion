#include "../x86_helpers.h"
#include "x86_instructions.h"

#include <stdbool.h>

bool X86_CaseMountCallReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder; 

    if (mir_node->src.type != HIDR_OPERAND_TYPE_REG) return false;

    FusHidrVirtualReg_t src_reg = X86_MapVirtualReg(mir_node->src.data.reg);

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

    /* REX só se registrador >= R8 (precisa REX.B) */
    if (src_reg > 7) {
        mount_instr->rex.w    = 0;
        mount_instr->rex.b    = 1;
        mount_instr->has_rex  = true;
    }

    return true;
}

bool X86_CaseMountCallImm(X86BackendContext* backend_ctx)
{
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    mount_instr->opcode.opcode[0] = 0xE8;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->imm.value = 0;
    mount_instr->imm.size  = 4; /* rel32 — sempre 4 bytes */
    mount_instr->has_imm   = true;

    return true;
}