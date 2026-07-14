#include <Fusion/Fusion.h>
#include <Fusion/FusionTrace.h>

#include <stdint.h>
#include <stdio.h>

typedef void (*FusionEntryPoint)(void);

static void Hello(void)
{
    puts("Executado via JIT!");
    fflush(stdout);
}

int main(void)
{
    FusInstance instance = NULL;
    FusCodeMount mount = NULL;
    FusLinkerContext linker = NULL;
    FusModuleBackend x86 = NULL;
    FusBackendReturn compiler = NULL;
    FusTraceTree_t trace = NULL;
    FusBufferContext_t *buffer = NULL;

    if (FUS_CreateInstance(&instance, NULL) != FUSION_OK) return 1;

    FUS_InstanceGetTrace(instance, &trace);
    FUS_CreateCodeMount(&instance, &mount);

    FUS_InsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(0),
            FUS_HIDR_Sym("Hello")));

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
    FUS_AddSymbolLinker(
        linker,
        "Hello",
        (uintptr_t)&Hello);

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

    FUS_MountHidrsBytes(
        &instance,
        (FusCommandRuleBase_t *)&hidr,
        &compiler);
    FusCommandBackend link = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .pNext = NULL,
        .backend = x86
    };

    FUS_LinkerResolver(
        (FusCommandRuleBase_t *)&link,
        linker,
        compiler);
    buffer = FUS_GetStreamBufferCompiler(compiler);

    printf("Code Bytes: ");
    for (size_t i = 0; i < buffer->offset; i++) {
        printf("%02X",buffer->buffer[i]);
    }
    printf("\n");

    if (FUS_ExecutableBuffer(buffer) == FUSION_OK) ((FusionEntryPoint)buffer->buffer)();
    FUS_DumpTrace(trace);

    FUS_DestroyBufferCode(buffer);
    FUS_DestroyCompiler(instance, compiler);
    FUS_DestroyBackend(x86);
    FUS_DestroyLinkerContext(instance, linker);
    FUS_DestroyCodeMount(&instance, mount);
    FUS_DestroyInstance(&instance);

    return 0;
}