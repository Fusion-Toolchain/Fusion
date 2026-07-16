#include <Fusion/Fusion.h>
#include <Fusion/FusionTrace.h>

#include <stdint.h>
#include <stdio.h>

typedef void (*FusionEntryPoint)(void);

void Print(int value)
{
    char buffer[32];
    snprintf(buffer, 32, "Valor para Função: %d\n", value);
    puts(buffer);
}

int main(void)
{
    FusInstance instance = NULL;
    FusCodeMount mount = NULL;
    FusLinkerContext linker = NULL;
    FusModuleBackend x86 = NULL;
    FusBackendReturn compiler = NULL;
    FusTraceTree trace = NULL;
    FusBufferContext_t *buffer = NULL;

    static uintptr_t Table[] = {
        (uintptr_t)&Print
    };

    if (FUS_CreateInstance(&instance, NULL) != FUSION_OK) return 1;
    FUS_InstanceGetTrace(instance, &trace);
    FUS_CreateCodeMount(&instance, &mount);

    FUS_InsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(1),
            FUS_HIDR_Sym("Table")));
    FUS_InsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(0),
            FUS_HIDR_Mem(1,0)));
    FUS_InsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(5),
            FUS_HIDR_Imm(30,HIDR_IMM64)));
    FUS_InsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_CALL,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(0),
            FUS_HIDR_None()));

    FUS_InsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_RET,
            HIDR_OP_SIZE_NONE,
            FUS_HIDR_None(),
            FUS_HIDR_None()));

    FUS_CreateLinkerContext(&instance, &linker);
    FUS_AddSymbolLinker(linker,"Table",(uintptr_t)&Table);

    FUS_LoaderBackend(
        &instance,
        &x86,
        "X86_Backend",
        FUS_BACKEND_TYPE_STATIC);
    FusCommandBackend backend = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .pNext = NULL,
        .backend = x86
    };
    FusCommandHidr hidr = {
        .sType = FUS_COMMAND_SEND_HIDR,
        .pNext = (FusCommandRuleBase_t *)&backend,
        .code = mount
    };

    if (FUS_MountHidrsBytes(
        &instance,
        (FusCommandRuleBase_t *)&hidr,
        &compiler) != FUSION_OK) {
            printf("Erro ao gerar codigo!\n");
            goto _end;
        }
    FusCommandBackend link = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .pNext = NULL,
        .backend = x86
    };

    if (FUS_LinkerResolver(
        (FusCommandRuleBase_t *)&link,
        linker,
        compiler) != FUSION_OK) {
            printf("Falha no Linker!\n");
            goto _end;
        }
    buffer = FUS_GetStreamBufferCompiler(compiler);

    printf("Code Bytes: ");
    for (size_t i = 0; i < buffer->offset; i++) {
        printf("%02X ",buffer->buffer[i]);
    }
    printf("\n");
    FUS_DumpTrace(trace);

    if (FUS_ExecutableBuffer(buffer) == FUSION_OK) ((FusionEntryPoint)buffer->buffer)();

_end:

    FUS_DestroyBufferCode(buffer);
    FUS_DestroyBackendReturn(instance, compiler);
    FUS_DestroyBackend(x86);
    FUS_DestroyLinkerContext(instance, linker);
    FUS_DestroyCodeMount(instance, mount);
    FUS_DestroyInstance(instance);

    return 0;
}