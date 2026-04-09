#include <Fusion/IRTypes/HidrType.h>
#include <Fusion/FusionTypes.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// LOCAL
#include "x86_functions.h"
#include "x86_types.h"

static inline size_t CalMirImmSize(FusHidrImmSize_t size_enum)
{
    switch (size_enum) {
        case HIDR_IMM8: return 1;
        case HIDR_IMM16: return 2;
        case HIDR_IMM32: return 4;
        case HIDR_IMM64: return 8;

        default: return 0;
    }
}

static bool X86_MountRet(FusHidrNode_t* mir_node,x86Instruction_t* mount_instr)
{
    mount_instr->opcode.opcode[0] = 0xC3;
    mount_instr->opcode.opcode_size = 1;

    return true;
}

static inline bool MirMovRules(FusHidrNode_t* mir_node)
{
    if (mir_node->mode == HIDR_MODE_NONE) return false;
    if (mir_node->src.type == HIDR_OPERAND_TYPE_IMM) return false;
    if (mir_node->src.type == HIDR_OPERAND_TYPE_MEM_REF) return false;

    return true;
}
static inline void X86_CaseMountMovRegImm(FusHidrNode_t* mir_node,x86Instruction_t* mount_instr)
{
    FusHidrImmSize_t imm_type = mir_node->dst.data.imm.size;

    if (imm_type == HIDR_IMM16) {
        mount_instr->prefix.prefix[0] = 0x66;
        mount_instr->prefix.prefix_size = 1;
        mount_instr->has_prefix = true;
    }

    mount_instr->opcode.opcode[0] = (imm_type == HIDR_IMM8 ? 0xB0 : 0xB8) + mir_node->src.data.reg;
    mount_instr->opcode.opcode_size = 1;
}
static bool X86_MountMov(FusHidrNode_t* mir_node,x86Instruction_t* mount_instr)
{
    if (!MirMovRules(mir_node)) return false;

    if (mir_node->dst.type == HIDR_OPERAND_TYPE_IMM) {
        X86_CaseMountMovRegImm(mir_node,mount_instr);

        size_t imm_size = CalMirImmSize(mir_node->dst.data.imm.size);
        if (!imm_size) return false;
        mount_instr->imm.value = mir_node->dst.data.imm.imm;
        mount_instr->imm.size = imm_size;
        mount_instr->has_imm = true;
    }
    if (mir_node->dst.type == HIDR_OPERAND_TYPE_REG) {
        if (mir_node->mode == HIDR_MODE16) {
            mount_instr->prefix.prefix[0] = 0x66;
            mount_instr->prefix.prefix_size = 1;
            mount_instr->has_prefix = true;
        }
        mount_instr->opcode.opcode[0] = 0x89;
        mount_instr->opcode.opcode_size = 1;

        mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT,
        mount_instr->modrm.reg = mir_node->dst.data.reg;
        mount_instr->modrm.rm = mir_node->src.data.reg;
        mount_instr->has_modrm = true;
    }
    return true;
}

static void X86_MirPrefixMount(FusHidrNode_t* mir_node, x86Instruction_t* instr_bytes)
{
    switch (mir_node->mode) {
        case HIDR_MODE64: {
            instr_bytes->prefix.prefix[0] = 0x48;
            instr_bytes->prefix.prefix_size = 1;
            instr_bytes->has_prefix = true;
        }
        default: return;
    }
}

typedef bool (*X86_MounterOpcodeFunc_t)(FusHidrNode_t*,x86Instruction_t*);
typedef struct {
    FusHidrNodeKind_t opcode;
    X86_MounterOpcodeFunc_t func;
} X86_OpcodeProcess_t;
static X86_OpcodeProcess_t opcode_table[] = {
    {HIDR_INSTR_RET, X86_MountRet},
    {HIDR_INSTR_MOV, X86_MountMov}
};

FusStatusFlag_t X86_BackendMountMir(FusBufferContext_t* fus_buffer, FusHidrNode_t* mir_node)
{
    if (!fus_buffer || !mir_node) return FUSION_ERRO;

    x86Instruction_t instr_x86 = {0};
    bool found_opcode = false;

    X86_MirPrefixMount(mir_node,&instr_x86);
    for (size_t i = 0; i < (sizeof(opcode_table) / sizeof(opcode_table[0])); i++) {
        if (opcode_table[i].opcode != mir_node->opcode) continue;
        if (!opcode_table[i].func(mir_node,&instr_x86)) return FUSION_INVALID_OPERAND;

        found_opcode = true;
        break;
    }
    if (!found_opcode) return FUSION_INVALID_OPCODE;
    
    if (
        X86_MountCodeBytes(&instr_x86,&fus_buffer->offset,fus_buffer->buffer,fus_buffer->buffer_size)
    ) return FUSION_OK; // X86 Mount return true.

    return FUSION_INVALID_OPCODE; // X86 Mount return false.
}


//     INTERFACE
#include <Internal/Fus_StaticBackend.h>

static FusBackendInterface_t interface = {
    .FUSI_BackendMountMir = X86_BackendMountMir
};
FusBackendInterface_t* X86_BackendDefine()
{
    return &interface;
}
REGISTER_BACKEND(X86_Backend,X86_BackendDefine);