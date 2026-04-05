/*
 *  !!! TEST FILE!!!
*/

#include <Fusion/Fusion.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static FusMirNode_t mir_exemple_8 = {
    .mode = MIR_MODE16,
    .opcode = MIR_INSTR_MOV,
    .src = {.type = MIR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = MIR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = MIR_IMM8}}
};
static FusMirNode_t mir_exemple_16 = {
    .mode = MIR_MODE16,
    .opcode = MIR_INSTR_MOV,
    .src = {.type = MIR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = MIR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = MIR_IMM16}}
};
static FusMirNode_t mir_exemple_32 = {
    .mode = MIR_MODE32,
    .opcode = MIR_INSTR_MOV,
    .src = {.type = MIR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = MIR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = MIR_IMM32}}
};
static FusMirNode_t mir_exemple_64 = {
    .mode = MIR_MODE64,
    .opcode = MIR_INSTR_MOV,
    .src = {.type = MIR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = MIR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = MIR_IMM64}}
};
static FusMirNode_t mir_exemple_reg_reg = {
    .mode = MIR_MODE16,
    .opcode = MIR_INSTR_MOV,
    .src = {.type = MIR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = MIR_OPERAND_TYPE_REG, .data.reg = 0x1}
};

static FusMirNode_t mir_exemple2 = {
    .opcode = MIR_INSTR_RET
};

static bool SaveBufferToFile(const char* filename, FusBufferContext_t* buf) {
    if (!buf || !buf->buffer || !filename) return false;

    FILE* f = fopen(filename, "wb");
    if (!f) return false;

    size_t written = fwrite(buf->buffer, 1, buf->offset, f);
    fclose(f);

    return written == buf->offset;
}

int main()
{
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

    FUS_MountMirBytes(buffer,&mir_exemple2);

    for (size_t i = 0; i < buffer->offset; i++) {
        printf(" %02X",buffer->buffer[i]);
    }
    printf("\n");

    SaveBufferToFile("out.bin",buffer);
    FUS_DestroyBufferCode(buffer);
}