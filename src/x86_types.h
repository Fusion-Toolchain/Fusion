#ifndef X86_BACKEND_TYPES
#define X86_BACKEND_TYPES
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t opcode[3];
    uint8_t opcode_size;
} x86Opcode_t;

#define MODRM_MOD_MEM_00            0x0  // endereço indireto, sem deslocamento (ou R/M=101 = absolute 32bits)
#define MODRM_MOD_MEM_8BIT_DISP     0x1  // endereço indireto + deslocamento 8 bits
#define MODRM_MOD_MEM_32BIT_DISP    0x2  // endereço indireto + deslocamento 32 bits
#define MODRM_MOD_REG_DIRECT        0x3  // registrador direto (MOD=11)

#define MODRM_RM_EAX 0x0
#define MODRM_RM_ECX 0x1
#define MODRM_RM_EDX 0x2
#define MODRM_RM_EBX 0x3
#define MODRM_RM_ESP 0x4  // se MOD!=11, SIB byte obrigatório
#define MODRM_RM_EBP 0x5  // MOD=00 → endereço absoluto de 32 bits
#define MODRM_RM_ESI 0x6
#define MODRM_RM_EDI 0x7

#define MODRM_RM_MAX_VALUE 7
#define MODRM_REG_MAX_VALUE 7
#define MODRM_MOD_MAX_VALUE 3
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
    x86Prefix_t prefix;
    x86Opcode_t opcode;
    x86ModRm_t modrm;

    bool has_modrm;
} x86Instruction_t;

#endif