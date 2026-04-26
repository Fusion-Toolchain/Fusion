#include "../x86_helpers.h"
#include "Internal/Fus_Backend.h"
#include "x86_instructions.h"
#include <stdbool.h>
#include <stddef.h>

bool X86_CaseMountMovImmReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    FusHidrImmSize_t imm_type = mir_node->src.data.imm.size;

    mount_instr->opcode.opcode[0] = (imm_type == HIDR_IMM8 ? 0xB0 : 0xB8) + X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->opcode.opcode_size = 1;
    size_t imm_size = X86_CalMirImmSize(mir_node->src.data.imm.size);

    if (!imm_size) return false;

    mount_instr->imm.value = mir_node->src.data.imm.imm;
    mount_instr->imm.size = imm_size;
    mount_instr->has_imm = true;

    return true;
}

bool X86_CaseMountMovRegReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    mount_instr->opcode.opcode[0] = 0x89;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT,
    mount_instr->modrm.reg = X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->modrm.rm = mir_node->src.data.reg;
    mount_instr->has_modrm = true;
    return true;
}

// TODO: X86_CaseMountMovMemImm New Case, Porfavor melhorar implementação!
bool X86_CaseMountMovMemImm(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* instr = backend_ctx->encoder;

    int reg_mem = X86_MapVirtualReg(mir_node->dst.data.memory_ref.base);
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

bool X86_CaseMountMovSymReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    const char* symbol_name = mir_node->src.data.sym.name;

    mount_instr->opcode.opcode[0] = 0xB8 + X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->opcode.opcode_size = 1;

    mount_instr->imm.value = 0xFFFFFFFFFFFFFFFF;
    mount_instr->imm.size = 8;
    mount_instr->has_imm = true;

    FusBufferContext_t* buffer = backend_ctx->block->buffer;
    size_t offset = buffer->offset + 2; // APONTA PARA IMM
    X86RegistreRealloc(backend_ctx->block,symbol_name,X86_ABS64,offset);
    return true;
}