/*
 *  Family Define: INC
 *
*/


static X86FamilyRule_t mov_rules[] = {
    [0]={
        .builder = X86_CaseMountMovImmReg,
        .dst_type = HIDR_OPERAND_TYPE_REG,
        .src_type = HIDR_OPERAND_TYPE_IMM
    },
    [1]={
        .builder = X86_CaseMountMovRegReg,
        .dst_type = HIDR_OPERAND_TYPE_REG,
        .src_type = HIDR_OPERAND_TYPE_REG
    },
    [2]={
        .builder = X86_CaseMountMovMemImm,
        .dst_type = HIDR_OPERAND_TYPE_MEM_REF,
        .src_type = HIDR_OPERAND_TYPE_IMM
    },
    [3]={
        .builder = X86_CaseMountMovSymReg,
        .dst_type = HIDR_OPERAND_TYPE_REG,
        .src_type = HIDR_OPERAND_TYPE_SYM
    }
};
static X86FamilyRule_t ret_rules[] = {
    [0]={
        .builder = X86_MountRet,
        .dst_type = HIDR_OPERAND_TYPE_NONE,
        .src_type = HIDR_OPERAND_TYPE_NONE
    }
};
static X86FamilyRule_t add_rules[] = {
    {HIDR_OPERAND_TYPE_REG, HIDR_OPERAND_TYPE_REG, X86_CaseMountAddRegReg},
    {HIDR_OPERAND_TYPE_IMM, HIDR_OPERAND_TYPE_REG, X86_CaseMountAddImmReg},
};
static X86FamilyRule_t lea_rules[] = {
    {HIDR_OPERAND_TYPE_MEM_REF, HIDR_OPERAND_TYPE_REG, X86_CaseMountLeaRegMem},
};
static X86FamilyRule_t call_rules[] = {
    {HIDR_OPERAND_TYPE_REG, HIDR_OPERAND_TYPE_NONE, X86_CaseMountCallReg},
    {HIDR_OPERAND_TYPE_SYM, HIDR_OPERAND_TYPE_NONE, X86_CaseMountCallImm}
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
    },
    [2]={.opcode=HIDR_INSTR_ADD, .rules=add_rules, .rule_count=2},
    [3]={.opcode=HIDR_INSTR_ADDR, .rules=lea_rules, .rule_count=1},
    [4]={.opcode=HIDR_INSTR_CALL, .rules=call_rules, .rule_count=2}
};