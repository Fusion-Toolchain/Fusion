#include "../x86_helpers.h"
#include "../x86_types.h"
#include "x86_instructions.h"

bool X86_CaseMountLeaRegMem(X86BackendContext* backend_ctx)
{
    const FusHidrNode_t* mir_node = backend_ctx->hidr;
    x86Instruction_t* mount_instr = backend_ctx->encoder;

    /* Validação de operands */
    if (mir_node->dst.type != HIDR_OPERAND_TYPE_REG) return false;
    if (mir_node->src.type != HIDR_OPERAND_TYPE_MEM_REF) return false;

    size_t dst_reg  = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->dst.data.reg));
    size_t base_reg = X86_MapVirtualReg(FUS_HIDR_REG_INTERNAL(mir_node->src.data.memory_ref.base));
    if (dst_reg == (size_t)-1 || base_reg == (size_t)-1) return false;

    uint16_t            offset   = mir_node->src.data.memory_ref.offset;
    /*
     * RSP (rm=100) como base sem SIB é encoding indefinido no x86.
     * Quando HIDR_OPERAND_TYPE_MEM_REF tiver index/scale, resolve aqui.
     */
    if ((base_reg & 0x7) == 0x4) { /* RSP ou R12 — ambos rm=100 */
        return false;
    }

    /* Opcode */
    mount_instr->opcode.opcode[0] = 0x8D;
    mount_instr->opcode.opcode_size = 1;

    /* ModRM
     *   mod=00 → [base]          (offset == 0, exceto RBP → forçamos mod=01)
     *   mod=01 → [base + disp8]
     *   mod=10 → [base + disp32]
     */
    uint8_t mod;
    if (offset == 0 && (base_reg & 0x7) != 0x5) {
        /* RBP/R13 com mod=00 vira RIP-relative — força disp8=0 */
        mod = MODRM_MOD_MEM_00;       /* 0b00 */
    } else if (offset <= 0x7F) {
        mod = MODRM_MOD_MEM_8BIT_DISP;         /* 0b01 */
    } else {
        mod = MODRM_MOD_MEM_32BIT_DISP;        /* 0b10 */
    }

    mount_instr->modrm.mod = mod;
    mount_instr->modrm.reg = (uint8_t)(dst_reg  & 0x7); /* reg  = destino  */
    mount_instr->modrm.rm  = (uint8_t)(base_reg & 0x7); /* r/m  = base     */
    mount_instr->has_modrm = true;

    /* REX.W (64-bit) + extensões de registrador */
    mount_instr->rex.w = 1;
    mount_instr->rex.r = (dst_reg  > 7) ? 1 : 0; /* dst  >= R8 → REX.R */
    mount_instr->rex.b = (base_reg > 7) ? 1 : 0; /* base >= R8 → REX.B */
    mount_instr->has_rex = true;

    /* Displacement */
    if (mod == MODRM_MOD_MEM_8BIT_DISP) {
        mount_instr->disp.value = (int32_t)offset;
        mount_instr->disp.size  = 1;
        mount_instr->has_disp   = true;
    } else if (mod == MODRM_MOD_MEM_32BIT_DISP) {
        mount_instr->disp.value = (int32_t)offset;
        mount_instr->disp.size  = 4;
        mount_instr->has_disp   = true;
    }

    return true;
}