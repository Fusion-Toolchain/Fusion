#include "../x86_helpers.h"
#include "../x86_types.h"
#include "x86_instructions.h"

bool X86_CaseMountSyscall(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;
    
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_NONE) return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_NONE) return false;
    
    // SYSCALL: 0F 05
    mount_instr->opcode.opcode[0] = 0x0F;
    mount_instr->opcode.opcode[1] = 0x05;
    mount_instr->opcode.opcode_size = 2;
    
    return true;
}