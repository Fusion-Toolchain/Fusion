#ifndef FUSION_MIR_TYPE_H
#define FUSION_MIR_TYPE_H
#include <stddef.h>
#include <stdint.h>

typedef size_t FusMirVirtualReg_t; 

typedef enum {
    MIR_INSTR_MOV,
    MIR_INSTR_ADD,
    MIR_INSTR_RET
} FusMirNodeKind_t;
typedef enum {
    MIR_OPERAND_TYPE_REG,
    MIR_OPERAND_TYPE_IMM
} FusMirOperandType_t;

typedef enum {
    MIR_IMM8,
    MIR_IMM16,
    MIR_IMM32,
    MIR_IMM64,
} FusMirImmSize_t;
typedef struct {
    uint64_t imm;
    FusMirImmSize_t size;
} FusMirImm_t;

typedef struct {
    FusMirOperandType_t type;
    union {
        FusMirVirtualReg_t reg;
        FusMirImm_t imm;
    } data;
} FusMirOperand_t;
typedef struct {
    FusMirNodeKind_t opcode;
    FusMirOperand_t src;
    FusMirOperand_t dst;
} FusMirNode_t;

#endif