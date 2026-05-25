#include "../x86_helpers.h"
#include "x86_instructions.h"

bool X86_MountRet(X86BackendContext* backend_ctx)
{
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    mount_instr->opcode.opcode[0] = 0xC3;
    mount_instr->opcode.opcode_size = 1;

    return true;
}
