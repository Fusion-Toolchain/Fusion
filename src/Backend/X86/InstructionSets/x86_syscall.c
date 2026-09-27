/*
 * x86_syscall.c — SYSCALL encoder (0F 05)
 */

#include "../x86_helpers.h"
#include "../x86_types.h"
#include "x86_instructions.h"

bool X86_CaseMountSyscall(X86BackendContext *ctx)
{
    const FusHidrNode_t *mir = ctx->hidr;
    x86Instruction_t *enc    = ctx->encoder;

    if (mir->dst.type != HIDR_OPERAND_TYPE_NONE) return false;
    if (mir->src.type != HIDR_OPERAND_TYPE_NONE) return false;

    // SYSCALL is 0F 05, no ModRM
    enc->opcode.opcode[0]   = 0x0F;
    enc->opcode.opcode[1]   = 0x05;
    enc->opcode.opcode_size = 2;

    return true;
}
