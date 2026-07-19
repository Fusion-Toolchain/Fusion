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
    
    mount_instr->opcode.opcode[0] = 0xB8 + X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->opcode.opcode_size = 1;
    
    // Se for 64-bit, SEMPRE precisa REX.W (mesmo com imm32)
    if (mir_node->op_size == HIDR_OP_SIZE_64) {
        mount_instr->rex.w = 1;
        mount_instr->has_rex = true;

        if (imm_type <= HIDR_IMM32) {
            mount_instr->imm.size = 4;  // imm32 sign-extended
        } else {
            mount_instr->imm.size = 8;  // imm64 completo
        }
    } else {
        // 32-bit (sem REX.W)
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

bool X86_CaseMountMovRegMem(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* instr = backend_ctx->encoder;

    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG) return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_MEM_REF) return false;
    FusHidrVirtualReg_t dst_reg = X86_MapVirtualReg(mir_node->dst.data.reg);
    FusHidrVirtualReg_t base_reg = X86_MapVirtualReg(mir_node->src.data.memory_ref.base);

    int32_t offset = mir_node->src.data.memory_ref.offset;
    /*
     * MOV r64, r/m64
     *
     * Opcode:
     *   48 8B /r
     */
    instr->opcode.opcode[0] = 0x8B;
    instr->opcode.opcode_size = 1;

    uint8_t mod;
    if (offset == 0 && (base_reg & 0x7) != 5) {
        mod = 0;        // [base]
    }
    else if (offset >= -128 && offset <= 127) {
        mod = 1;        // [base + disp8]
    }
    else {
        mod = 2;        // [base + disp32]
    }

    instr->modrm.mod = mod;
    instr->modrm.reg = dst_reg & 0x7;
    instr->modrm.rm  = base_reg & 0x7;
    instr->has_modrm = true;

    /*
     * REX.W + REX.R + REX.B
     */

    instr->rex.w = 1;
    instr->rex.r = (dst_reg >= 8);
    instr->rex.b = (base_reg >= 8);
    instr->has_rex = true;

    /*
     * displacement
     */

    if (mod == 1) {
        instr->disp.value = offset;
        instr->disp.size = 1;
        instr->has_disp = true;
    }
    else if (mod == 2) {
        instr->disp.value = offset;
        instr->disp.size = 4;
        instr->has_disp = true;
    }

    return true;
}

bool X86_CaseMountMovSymReg(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node   = backend_ctx->hidr;
    x86Instruction_t*    mount_instr = backend_ctx->encoder;
    const char*          symbol_name = mir_node->src.data.sym.name;

    mount_instr->opcode.opcode[0]  = 0xB8 + X86_MapVirtualReg(mir_node->dst.data.reg);
    mount_instr->opcode.opcode_size = 1;
    mount_instr->imm.value          = 0xFFFFFFFFFFFFFFFF;
    mount_instr->imm.size           = 8;
    mount_instr->has_imm            = true;

    size_t offset = backend_ctx->block->slab_offset + 2; // aponta pro IMM

    FUSB_REGISTRE_REALOCATION(backend_ctx->Api,backend_ctx->block,symbol_name,X86_ABS64,offset);
    return true;
}