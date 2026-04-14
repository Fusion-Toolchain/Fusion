/*
 *  !!! TEST FILE!!!
*/

#include "Fusion/IRTypes/HidrType.h"
#include <Fusion/Fusion.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static FusHidrNode_t mir_exemple_8 = {
    .mode = HIDR_MODE16,
    .op_size = HIDR_OP_SIZE_8,
    .opcode = HIDR_INSTR_MOV,
    .src = {.type = HIDR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = HIDR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = HIDR_IMM8}}
};
static FusHidrNode_t mir_exemple_16 = {
    .mode = HIDR_MODE16,
    .op_size = HIDR_OP_SIZE_16,
    .opcode = HIDR_INSTR_MOV,
    .src = {.type = HIDR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = HIDR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = HIDR_IMM16}}
};
static FusHidrNode_t mir_exemple_32 = {
    .mode = HIDR_MODE32,
    .op_size = HIDR_OP_SIZE_32,
    .opcode = HIDR_INSTR_MOV,
    .src = {.type = HIDR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = HIDR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = HIDR_IMM32}}
};
static FusHidrNode_t mir_exemple_64 = {
    .mode = HIDR_MODE64,
    .op_size = HIDR_OP_SIZE_64,
    .opcode = HIDR_INSTR_MOV,
    .src = {.type = HIDR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = HIDR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = HIDR_IMM64}}
};
static FusHidrNode_t mir_exemple_reg_reg = {
    .mode = HIDR_MODE32,
    .op_size = HIDR_OP_SIZE_32,
    .opcode = HIDR_INSTR_MOV,
    .src = {.type = HIDR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = HIDR_OPERAND_TYPE_REG, .data.reg = 0x1}
};

static FusHidrNode_t mir_memory_ref = {
    .mode = HIDR_MODE64,
    .op_size = HIDR_OP_SIZE_64,
    .opcode = HIDR_INSTR_MOV,
    .src = {.type = HIDR_OPERAND_TYPE_MEM_REF, .data.memory_ref = {
        .base = HIDR_VREG_STACK_PTR,
        .offset = 8
    }},
    .dst = {.type = HIDR_OPERAND_TYPE_IMM, .data.imm = {
        .imm = 0xFF,
        .size = HIDR_IMM64
    }}
};

static FusHidrNode_t mir_exemple2 = {
    .opcode = HIDR_INSTR_RET,
};

static FusFileManagerSectionDefine_t text_section = {
    .alignment = 0,
    .name = "Text",
    .offset = 0,
    .size = 100,
    .type = FUS_FILE_SECTION_TYPE_READ | FUS_FILE_SECTION_TYPE_EXEC,
};

int main()
{
    FusFileManager_t* file = FUS_CreateFileDevice("out.bin");
    if (!file) {
        printf("Erro ao criar arquivo!\n");
        return 1;
    }
    FUS_FileSectionAdd(file,&text_section);
    FusBufferContext_t* buffer = FUS_CreateBufferCode(1*1024);
    if (!buffer) {
        printf("Erro: Erro to build buffer Fusion!\n");
        return 1;
    }

    FusStatusFlag_t st = FUS_MountMirBytes(buffer,&mir_exemple_8);
    if (st != FUSION_OK) {
        printf("Erro ao gerar codigo! %s\n",FUS_StrError(st));
        FUS_DestroyBufferCode(buffer);
        return 1;
    }
    FUS_MountMirBytes(buffer,&mir_exemple_16);
    FUS_MountMirBytes(buffer,&mir_exemple_32);
    FUS_MountMirBytes(buffer,&mir_exemple_64);
    FUS_MountMirBytes(buffer,&mir_exemple_reg_reg);
    FUS_MountMirBytes(buffer,&mir_memory_ref);

    FUS_MountMirBytes(buffer,&mir_exemple2);

    FUS_SaveFileDeviceForBuffer(file,buffer);

    FUS_DestroyBufferCode(buffer);
    FUS_DestroyFileDevice(file);
}