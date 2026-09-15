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
    },
    [4]={
        .builder = X86_CaseMountMovRegMem,
        .dst_type = HIDR_OPERAND_TYPE_REG,
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
static X86FamilyRule_t add_rules[] = {
    {.dst_type = HIDR_OPERAND_TYPE_REG, .src_type = HIDR_OPERAND_TYPE_REG, .builder = X86_CaseMountAddRegReg},
    {.dst_type = HIDR_OPERAND_TYPE_REG, .src_type = HIDR_OPERAND_TYPE_IMM, .builder = X86_CaseMountAddImmReg},
};
static X86FamilyRule_t lea_rules[] = {
    {
        .src_type = HIDR_OPERAND_TYPE_MEM_REF,
        .dst_type = HIDR_OPERAND_TYPE_REG,
        .builder  = X86_CaseMountLeaRegMem
    }
};
static X86FamilyRule_t call_rules[] = {
    [0]={ .src_type=HIDR_OPERAND_TYPE_NONE, .dst_type=HIDR_OPERAND_TYPE_REG, .builder=X86_CaseMountCallReg },
    [1]={ .src_type=HIDR_OPERAND_TYPE_NONE, .dst_type=HIDR_OPERAND_TYPE_SYM, .builder=X86_CaseMountCallRel32 }
};
static X86FamilyRule_t push_rules[] = {
    [0]={
        .src_type = HIDR_OPERAND_TYPE_NONE,
        .dst_type = HIDR_OPERAND_TYPE_REG,
        .builder = X86_CaseMountPushReg
    },
    [1]={
        .src_type = HIDR_OPERAND_TYPE_NONE,
        .dst_type = HIDR_OPERAND_TYPE_IMM,
        .builder = X86_CaseMountPushImm
    }
};
static X86FamilyRule_t pop_rules[] = {
    [0]={
        .src_type = HIDR_OPERAND_TYPE_NONE,
        .dst_type = HIDR_OPERAND_TYPE_REG,
        .builder = X86_CaseMountPopReg
    },
    [1]={
        .src_type = HIDR_OPERAND_TYPE_NONE,
        .dst_type = HIDR_OPERAND_TYPE_IMM,
        .builder = X86_CaseMountPopImm
    }
};
static X86FamilyRule_t syscall_rules[] = {
    [0]={
        .src_type = HIDR_OPERAND_TYPE_NONE,
        .dst_type = HIDR_OPERAND_TYPE_NONE,
        .builder = X86_CaseMountSyscall
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
    },
    [2]={.opcode=HIDR_INSTR_ADD, .rules=add_rules, .rule_count=2},
    [3]={.opcode=HIDR_INSTR_ADDR, .rules=lea_rules, .rule_count=1},
    [4]={.opcode=HIDR_INSTR_CALL, .rules=call_rules, .rule_count=2},
    [5]={.opcode=HIDR_INSTR_PUSH, .rules=push_rules, .rule_count=2},
    [6]={.opcode=HIDR_INSTR_POP, .rules=pop_rules, .rule_count=2},
    [7]={.opcode=HIDR_INSTR_SYSCALL, .rules=syscall_rules, .rule_count=1}
};