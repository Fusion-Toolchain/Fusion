#ifndef X86_BACKEND_TYPES
#define X86_BACKEND_TYPES

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    X86_REL32,
    X86_ABS64
} X86ReallocTypes_t;

// REGISTERS (único source of truth)
#define X86_REG_RAX 0
#define X86_REG_RCX 1
#define X86_REG_RDX 2
#define X86_REG_RBX 3
#define X86_REG_RSP 4
#define X86_REG_RBP 5
#define X86_REG_RSI 6
#define X86_REG_RDI 7

/*
 * ! MODRM TYPES !
*/

#define MODRM_MOD_MEM_00            0x0  
#define MODRM_MOD_MEM_8BIT_DISP     0x1
#define MODRM_MOD_MEM_32BIT_DISP    0x2
#define MODRM_MOD_REG_DIRECT        0x3

/*
 * ! MODRM MAX VALUES !
*/

#define MODRM_RM_MAX_VALUE 7
#define MODRM_REG_MAX_VALUE 7
#define MODRM_MOD_MAX_VALUE 3

// SIB
#define X86_SIB_INDEX_NONE 4

#define X86_SIB_SCALE_1 0
#define X86_SIB_SCALE_2 1
#define X86_SIB_SCALE_4 2
#define X86_SIB_SCALE_8 3

typedef struct {
    uint8_t opcode[3];
    uint8_t opcode_size;
} x86Opcode_t;

typedef struct {
    uint8_t mod;
    uint8_t reg;
    uint8_t rm;
} x86ModRm_t;

typedef struct {
    uint8_t prefix[4];
    uint8_t prefix_size;
} x86Prefix_t;
typedef struct {
    uint64_t value;
    uint8_t size;
} x86Imm_t;
typedef struct {
    uint8_t scale;
    uint8_t index;
    uint8_t base;
} x86Sib_t;
typedef struct {
    int32_t value;
    uint8_t size;
} x86Disp_t;
typedef struct {
    uint8_t w : 1;
    uint8_t r : 1;
    uint8_t x : 1;
    uint8_t b : 1;
} x86Rex_t;

typedef struct {
    x86Prefix_t prefix;
    x86Rex_t rex;
    x86Opcode_t opcode;
    x86ModRm_t modrm;
    x86Imm_t imm;
    x86Sib_t sib;
    x86Disp_t disp;

    bool has_prefix;
    bool has_modrm;
    bool has_imm;
    bool has_sib;
    bool has_disp;
    bool has_rex;
} x86Instruction_t;

#endif