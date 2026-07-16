#include <Fusion/Fusion.h>
#include <stdio.h>

void MountCode(FusCodeMount Mount)
{
    FUS_InsertCodeBlock(Mount,
        FUS_HIDRM(HIDR_INSTR_RET,
            HIDR_OP_SIZE_NONE,
            FUS_HIDR_None(),
            FUS_HIDR_None()));
}

int main(void)
{
    FusCodeMount CodeMount = NULL;
    FusInstance Instance = NULL;
    if (!FUS_CreateInstance(&Instance,NULL)) {
        printf("Erro Init Instace!\n");
        return 1;
    }
    if (!FUS_CreateCodeMount(&Instance,&CodeMount)) {
        printf("Erro Init CodeMount System\n");
        return 1;
    }

    MountCode(CodeMount);

    FusModuleBackend BackendInstance = NULL;
    if (!FUS_LoaderBackend(&Instance,&BackendInstance,"X86_Backend",FUS_BACKEND_TYPE_STATIC)) {
        printf("Erro Init BackendModule\n");
        return 1;
    }
    FusCommandBackend BackendChain = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .backend = BackendInstance,
        .pNext = NULL
    };
    FusCommandHidr HidrChain = {
        .sType = FUS_COMMAND_SEND_HIDR,
        .code = CodeMount,
        .pNext = (FusCommandRuleBase_t*)&BackendChain
    };

    FusBackendReturn BackendReturn = NULL;
    if (!FUS_MountHidrsBytes(&Instance,(FusCommandRuleBase_t*)&HidrChain,&BackendReturn)) {
        printf("Erro Compile Code\n");
        return 1;
    }
    FusBufferContext_t* CodeBuffer = FUS_GetStreamBufferCompiler(BackendReturn);
    if (FUS_ExecutableBuffer(CodeBuffer)) {
        void (*func)(void) = (void(*)(void))CodeBuffer->buffer;
        func();
    }

    FUS_DestroyBufferCode(CodeBuffer);
    FUS_DestroyBackendReturn(Instance,BackendReturn);
    FUS_DestroyBackend(BackendInstance);
    FUS_DestroyCodeMount(Instance,CodeMount);
    FUS_DestroyInstance(Instance);
}