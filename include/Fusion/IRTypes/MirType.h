#ifndef FUSION_MIR_TYPE_H
#define FUSION_MIR_TYPE_H
#include <stddef.h>
#include <stdint.h>

typedef size_t FusMirVirtualReg_t; 

/*
 * @brief Mir Opcode Types
*/
typedef enum {
    MIR_INSTR_MOV,
    MIR_INSTR_ADD,
    MIR_INSTR_RET
} FusMirNodeKind_t;
/*
 * @brief Mir Operand Types
*/
typedef enum {
    MIR_OPERAND_TYPE_REG,
    MIR_OPERAND_TYPE_IMM,
    MIR_OPERAND_TYPE_MEM,
} FusMirOperandType_t;

/*
 * @brief Mir Imm Size Types
 * @note Future Remove
*/
typedef enum {
    MIR_IMM8,
    MIR_IMM16,
    MIR_IMM32,
    MIR_IMM64,
} FusMirImmSize_t;
/*
 * @brief Mir Imm Struct Type
*/
typedef struct {
    uint64_t imm;
    FusMirImmSize_t size;
} FusMirImm_t;

/*
 * @brief Mir Operand Struct Type
*/
typedef struct {
    FusMirOperandType_t type;
    union {
        FusMirVirtualReg_t reg;
        FusMirImm_t imm;
    } data;
} FusMirOperand_t;

/*
 * @brief Mir Node Mode Types
*/
typedef enum {
    MIR_MODE_NONE = 0,
    MIR_MODE64,
    MIR_MODE32,
    MIR_MODE16,
} FusMirOpcodeMode_t;

/*
 * @brief Mir Node Struct
*/
typedef struct {
    FusMirOpcodeMode_t mode;

    FusMirNodeKind_t opcode;
    FusMirOperand_t src;
    FusMirOperand_t dst;
} FusMirNode_t;

#endif