#ifndef FUSION_HIDR_TYPE_H
#define FUSION_HIDR_TYPE_H

#include <stddef.h>
#include <stdint.h>

#define HIDR_VREG_FRAME_PTR (int16_t)-1
#define HIDR_VREG_STACK_PTR (int16_t)-2
#define HIDR_VREG_PC (int16_t)-3

#define HIDR_VREG_GENERAL_BASE   0    // 0  a 23
#define HIDR_VREG_SPECIAL_BASE   24   // 24 a 31
#define HIDR_VREG_STACK_BASE     32   // 32 a 35

#define HIDR_VREG_GENERAL_COUNT  24
#define HIDR_VREG_SPECIAL_COUNT  8
#define HIDR_VREG_STACK_COUNT    4

typedef enum {
    HIDR_VREG_CLASS_VIRTUAL,
    HIDR_VREG_CLASS_GENERAL,
    HIDR_VREG_CLASS_SPECIAL,
    HIDR_VREG_CLASS_STACK,
} FusHidrRegClass_t;

/*
 * @brief Hidr Virtual/Fisical Registre Represent (HIDR)
 * 
 * Negativos - Registradores universais.
 * Possitivos - Especificos da arquitetura.
*/
typedef int16_t FusHidrVirtualReg_t;


/*
 * @brief Hidr Opcode Types
*/
typedef enum {
    HIDR_INSTR_MOV,
    HIDR_INSTR_ADD,
    HIDR_INSTR_RET
} FusHidrNodeKind_t;
/*
 * @brief Hidr Operand Types
*/
typedef enum {
    HIDR_OPERAND_TYPE_REG,
    HIDR_OPERAND_TYPE_IMM,
    HIDR_OPERAND_TYPE_MEM_REF,
} FusHidrOperandType_t;

/*
 * @brief Hidr Imm Size Types
 * @note Future Remove
*/
typedef enum {
    HIDR_IMM8,
    HIDR_IMM16,
    HIDR_IMM32,
    HIDR_IMM64,
} FusHidrImmSize_t;
/*
 * @brief Hidr Imm Struct Type
*/
typedef struct {
    uint64_t imm;
    FusHidrImmSize_t size;
} FusHidrImm_t;
typedef struct {
    FusHidrVirtualReg_t base;
    uint16_t offset;
} FusHidrMemRef_t;

/*
 * @brief Hidr Operand Struct Type
*/
typedef struct {
    FusHidrOperandType_t type;
    union {
        FusHidrVirtualReg_t reg;
        FusHidrMemRef_t memory_ref;
        FusHidrImm_t imm;
    } data;
} FusHidrOperand_t;

/*
 * @brief Hidr Node Mode Types
*/
typedef enum {
    HIDR_MODE_NONE = 0,
    HIDR_MODE64,
    HIDR_MODE32,
    HIDR_MODE16,
} FusHidrOpcodeMode_t;

/*
 * @brief Hidr Node Struct
*/
typedef struct {
    FusHidrOpcodeMode_t mode;

    FusHidrNodeKind_t opcode;
    FusHidrOperand_t src;
    FusHidrOperand_t dst;
} FusHidrNode_t;

#endif