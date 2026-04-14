#include <Fusion/IRTypes/HidrType.h>
#include <Fusion/FusionTypes.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// LOCAL
#include "x86_functions.h"
#include "x86_types.h"

typedef bool (*X86FmailyRuleFunc_t)(FusHidrNode_t*,x86Instruction_t*);
typedef struct {
    FusHidrOperandType_t src_type;
    FusHidrOperandType_t dst_type;

    X86FmailyRuleFunc_t builder;
} X86FamilyRule_t;
typedef struct {
    X86FamilyRule_t* rules;
    size_t rule_count;

    FusHidrNodeKind_t opcode;
} x86Familys_t;

static inline size_t X86_CalMirImmSize(FusHidrImmSize_t size_enum)
{
    switch (size_enum) {
        case HIDR_IMM8: return 1;
        case HIDR_IMM16: return 2;
        case HIDR_IMM32: return 4;
        case HIDR_IMM64: return 8;

        default: return 0;
    }
}
// TODO: Pre Implementação da janela de Virtual Registres.
static inline size_t X86_MapVirtualReg(FusHidrVirtualReg_t reg)
{
    switch (reg) {
        case HIDR_VREG_STACK_PTR: return X86_REG_RSP;
        case HIDR_VREG_FRAME_PTR: return X86_REG_RBP;

        default: return reg;
    }
}

static bool X86_MountRet(FusHidrNode_t* mir_node,x86Instruction_t* mount_instr)
{
    mount_instr->opcode.opcode[0] = 0xC3;
    mount_instr->opcode.opcode_size = 1;

    return true;
}

static inline bool X86_CaseMountMovImmReg(FusHidrNode_t* mir_node,x86Instruction_t* mount_instr)
{
    FusHidrImmSize_t imm_type = mir_node->dst.data.imm.size;

    mount_instr->opcode.opcode[0] = (imm_type == HIDR_IMM8 ? 0xB0 : 0xB8) + mir_node->src.data.reg;
    mount_instr->opcode.opcode_size = 1;

    size_t imm_size = X86_CalMirImmSize(mir_node->dst.data.imm.size);
    if (!imm_size) return false;
    mount_instr->imm.value = mir_node->dst.data.imm.imm;
    mount_instr->imm.size = imm_size;
    mount_instr->has_imm = true;

    return true;
}
static bool X86_CaseMountMovRegReg(FusHidrNode_t* mir_node,x86Instruction_t* mount_instr)
{

    mount_instr->opcode.opcode[0] = 0x89;
    mount_instr->opcode.opcode_size = 1;

    mount_instr->modrm.mod = MODRM_MOD_REG_DIRECT,
    mount_instr->modrm.reg = mir_node->dst.data.reg;
    mount_instr->modrm.rm = mir_node->src.data.reg;
    mount_instr->has_modrm = true;
    return true;
}

// TODO: New Case, Porfavor melhorar implementação!
static bool X86_CaseMountMovMemImm(FusHidrNode_t* mir_node,x86Instruction_t* instr)
{
    int reg_mem = X86_MapVirtualReg(mir_node->src.data.memory_ref.base);
    int32_t offset = mir_node->src.data.memory_ref.offset;

    instr->opcode.opcode[0] = 0xC7;
    instr->opcode.opcode_size = 1;

    instr->modrm.reg = 0; // obrigatório

    // DISP
    if (offset >= -128 && offset <= 127) {
        instr->disp.size = 1;
        instr->modrm.mod = 1;
    } else {
        instr->disp.size = 4;
        instr->modrm.mod = 2;
    }

    instr->disp.value = offset;
    instr->has_disp = true;

    // SIB (só se precisar)
    if (reg_mem == 4) { // RSP
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

    // IMM (sempre 32-bit)
    instr->imm.value = mir_node->dst.data.imm.imm;
    instr->imm.size = 4;
    instr->has_imm = true;

    return true;
}

// TODO: Mover definição de familias, para um arquivo especial!
static X86FamilyRule_t mov_rules[] = {
    [0]={
        .builder = X86_CaseMountMovImmReg,
        .dst_type = HIDR_OPERAND_TYPE_IMM,
        .src_type = HIDR_OPERAND_TYPE_REG
    },
    [1]={
        .builder = X86_CaseMountMovRegReg,
        .dst_type = HIDR_OPERAND_TYPE_REG,
        .src_type = HIDR_OPERAND_TYPE_REG
    },
    [2]={
        .builder = X86_CaseMountMovMemImm,
        .dst_type = HIDR_OPERAND_TYPE_IMM,
        .src_type = HIDR_OPERAND_TYPE_MEM_REF
    }
};
static X86FamilyRule_t ret_rules[] = {
    [0]={
        .builder = X86_MountRet,
        .dst_type = HIDR_OPERAND_TYPE_NONE,
        .src_type = HIDR_OPERAND_TYPE_NONE
    }
};
static x86Familys_t familys[] = {
    [0]={
        .opcode = HIDR_INSTR_MOV,
        .rules = mov_rules,
        .rule_count = sizeof(mov_rules) / sizeof(mov_rules[0])
    },
    [1]={
        .opcode = HIDR_INSTR_RET,
        .rules = ret_rules,
        .rule_count = sizeof(ret_rules) / sizeof(ret_rules[0])
    }
};

static inline void X86_MountPrefixHidr(FusHidrNode_t* mir_node,x86Instruction_t* mount_instr)
{
    switch (mir_node->mode) {
        case HIDR_MODE16: {
            if (mir_node->op_size == HIDR_OP_SIZE_16) {
                mount_instr->prefix.prefix[0] = 0x66;
                mount_instr->prefix.prefix_size = 1;
                mount_instr->has_prefix = true;
            }
            break;
        }
        case HIDR_MODE64: {
            if (mir_node->op_size == HIDR_OP_SIZE_64) {
                mount_instr->prefix.prefix[0] = 0x48;
                mount_instr->prefix.prefix_size = 1;
                mount_instr->has_prefix = true;
            }
            break;
        }
        case HIDR_MODE32:
        default:
            return;
    }
}

static inline bool X86_SelectFamily(FusHidrNode_t* mir_node, x86Instruction_t* instr_x86)
{
    for (size_t i = 0; i < (sizeof(familys) / sizeof(familys[0])); i++) {
        if (familys[i].opcode != mir_node->opcode) continue;
        for (size_t j = 0; j < familys[i].rule_count; j++) {
            X86FamilyRule_t* rules = &familys[i].rules[j];
            if (rules->dst_type != mir_node->dst.type) continue;
            if (rules->src_type != mir_node->src.type) continue;

            if (rules->builder(mir_node,instr_x86)) return true;
            return false;
        }
        return false;
    }
    return true;
}

static FusStatusFlag_t X86_BackendMountMir(FusBufferContext_t* fus_buffer, FusHidrNode_t* mir_node)
{
    if (!fus_buffer || !mir_node) return FUSION_ERRO;

    x86Instruction_t instr_x86 = {0};
    X86_MountPrefixHidr(mir_node,&instr_x86);
    X86_SelectFamily(mir_node,&instr_x86);
    
    if (
        X86_MountCodeBytes(&instr_x86,&fus_buffer->offset,fus_buffer->buffer,fus_buffer->buffer_size)
    ) return FUSION_OK; // X86 Mount return true.

    return FUSION_INVALID_OPCODE; // X86 Mount return false.
}


//     INTERFACE DEFINE
#include <Internal/Fus_StaticBackend.h>

static FusBackendInterface_t interface = {
    .FUSI_BackendMountMir = X86_BackendMountMir
};
FusBackendInterface_t* X86_BackendDefine()
{
    return &interface;
}
REGISTER_BACKEND(X86_Backend,X86_BackendDefine);