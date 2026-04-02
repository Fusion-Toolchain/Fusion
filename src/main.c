#include "Fusion/FusionTypes.h"
#include "Fusion/IRTypes/MirType.h"
#include <Fusion/Fusion.h>

#include <stddef.h>
#include <stdio.h>

static FusMirNode_t mir_exemple = {
    .opcode = MIR_INSTR_MOV,
    .src = {.type = MIR_OPERAND_TYPE_REG, .data.reg = 0x0},
    .dst = {.type = MIR_OPERAND_TYPE_IMM, .data.imm = {.imm = 0xFF, .size = MIR_IMM64}}
};
static FusMirNode_t mir_exemple2 = {
    .opcode = MIR_INSTR_RET
};


int main()
{
    FusionBufferContext_t* buffer = FUS_CreateBufferCode(1*1024);
    if (!buffer) {
        printf("Erro: Erro to build buffer Fusion!\n");
        return 1;
    }

    if (FUS_MountMirBytes(buffer,&mir_exemple) != FUSION_OK) {
        printf("Erro ao gerar codigo!\n");
        FUS_DestroyBufferCode(buffer);
        return 1;
    }
    FUS_MountMirBytes(buffer,&mir_exemple2);

    for (size_t i = 0; i < buffer->offset; i++) {
        printf(" %02X",buffer->buffer[i]);
    }
    printf("\n");

    FUS_DestroyBufferCode(buffer);
}