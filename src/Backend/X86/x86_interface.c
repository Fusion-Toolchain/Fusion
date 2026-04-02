/*
   TODO: Mover este esquema para core do Fusion, implementar interface de backend para este casso.
*/

#include <Fusion/IRTypes/MirType.h>
#include <Fusion/FusionTypes.h>
#include <stddef.h>
#include <stdint.h>

#include "x86_functions.h"
#include "x86_types.h"

static inline size_t CalMirImmSize(FusMirImmSize_t size_enum)
{
    switch (size_enum) {
        case MIR_IMM8: return 1;
        case MIR_IMM16: return 2;
        case MIR_IMM32: return 4;
        case MIR_IMM64: return 8;

        default: return 0;
    }
}

static inline bool MirRetRules(FusMirNode_t* mir_node) // REGRA DO RET
{
    if (mir_node->opcode != MIR_INSTR_RET) return true;

    return (
        mir_node->src.type == MIR_OPERAND_TYPE_REG ||
        mir_node->src.type == MIR_OPERAND_TYPE_IMM
    );
}
static bool X86_MountRet(FusMirNode_t* mir_node,x86Instruction_t* mount_instr)
{
    if (!MirRetRules(mir_node)) return false;
    mount_instr->opcode.opcode[0] = 0xC3;
    mount_instr->opcode.opcode_size = 1;

    return true;
}

static inline bool MirMovRules(FusMirNode_t* mir_node)
{
    if (mir_node->opcode != MIR_INSTR_MOV) return true;

    return (
        mir_node->src.type == MIR_OPERAND_TYPE_REG ||
        mir_node->src.type == MIR_OPERAND_TYPE_IMM
    );
}
static inline void X86_CaseMountMovRegImm(FusMirNode_t* mir_node,x86Instruction_t* mount_instr)
{
    FusMirImmSize_t imm_type = mir_node->dst.data.imm.size;
    
    mount_instr->has_prefix = false;

    if (imm_type == MIR_IMM64) {
        mount_instr->prefix.prefix[0] = 0x48;
        mount_instr->prefix.prefix_size = 1;
        mount_instr->has_prefix = true;
    } else if (imm_type == MIR_IMM16) {
        mount_instr->prefix.prefix[0] = 0x66;
        mount_instr->prefix.prefix_size = 1;
        mount_instr->has_prefix = true;
    }

    mount_instr->opcode.opcode[0] = (imm_type == MIR_IMM8 ? 0xB0 : 0xB8) + mir_node->src.data.reg;
    mount_instr->opcode.opcode_size = 1;
}
static bool X86_MountMov(FusMirNode_t* mir_node,x86Instruction_t* mount_instr)
{
    if (!MirMovRules(mir_node)) return false;

    if (mir_node->dst.type == MIR_OPERAND_TYPE_IMM) {
        X86_CaseMountMovRegImm(mir_node,mount_instr);

        size_t imm_size = CalMirImmSize(mir_node->dst.data.imm.size);
        if (!imm_size) return false;

        mount_instr->imm.value = mir_node->dst.data.imm.imm;
        mount_instr->imm.size = imm_size;
        mount_instr->has_imm = true;
    }

    return true;
}

typedef bool (*X86_MounterOpcodeFunc_t)(FusMirNode_t*,x86Instruction_t*);
typedef struct {
    FusMirNodeKind_t opcode;
    X86_MounterOpcodeFunc_t func;
} X86_OpcodeProcess_t;

static X86_OpcodeProcess_t opcode_table[] = {
    {MIR_INSTR_RET, X86_MountRet},
    {MIR_INSTR_MOV, X86_MountMov}
};

FusionStatusFlag_t FUS_MountMirBytes(FusionBufferContext_t* fus_buffer, FusMirNode_t* mir_node)
{
    if (!fus_buffer || !mir_node) return FUSION_ERRO;

    x86Instruction_t instr_x86 = {0};
    bool found_opcode = false;

    for (size_t i = 0; i < (sizeof(opcode_table) / sizeof(opcode_table[0])); i++) {
        if (opcode_table[i].opcode != mir_node->opcode) continue;
        if (!opcode_table[i].func(mir_node,&instr_x86)) return FUSION_ERRO;

        found_opcode = true;
        break;
    }
    if (!found_opcode) return FUSION_ERRO;
    
    if (
        X86_MountCodeBytes(&instr_x86,&fus_buffer->offset,fus_buffer->buffer,fus_buffer->buffer_size)
    ) return FUSION_OK; // X86 Mount return true.

    return FUSION_ERRO; // X86 Mount return false.
}