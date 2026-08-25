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

static void MountCode(FusCodeMount mount)
{
    fusInsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(1),
            FUS_HIDR_Sym("Print"))
    );
    fusInsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(5),
            FUS_HIDR_Imm(30,HIDR_IMM64))
    );
    fusInsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_CALL,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(1),
            FUS_HIDR_None())
    );

    fusInsertCodeBlock(mount,
        FUS_HIDRM(HIDR_INSTR_RET,
            HIDR_OP_SIZE_NONE,
            FUS_HIDR_None(),
            FUS_HIDR_None())
    );
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

    if (!fusCreateInstance(&instance, NULL)) return 1;
    fusInstanceGetTrace(instance, &trace);
    fusCreateCodeMount(&instance, &mount);

    MountCode(mount); // CREATE CODE

    fusCreateLinkerContext(instance, &linker);
    fusAddSymbolLinker(linker,"Print",(uintptr_t)&Print);

    fusLoaderBackend(
        instance,
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

    if (!fusMountHidrsBytes(
        instance,
        (FusCommandRuleBase_t *)&hidr,
        &compiler)) {
            printf("Erro ao gerar codigo!\n");
            goto _end;
        }
    FusCommandBackend link = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .pNext = NULL,
        .backend = x86
    };

    if (!fusLinkerResolver(
        (FusCommandRuleBase_t *)&link,
        linker,
        compiler)) {
            printf("Falha no Linker!\n");
            goto _end;
        }
    buffer = fusGetStreamBufferCompiler(compiler);
    fusDumpTrace(trace);
    if (fusExecutableBuffer(buffer)) ((FusionEntryPoint)buffer->buffer)();

_end:
    fusDestroyBufferCode(buffer);
    fusDestroyBackendReturn(instance, compiler);
    fusDestroyBackend(x86);
    fusDestroyLinkerContext(instance, linker);
    fusDestroyCodeMount(instance, mount);
    fusDestroyInstance(instance);

    return 0;
}