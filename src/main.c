/*
 *  !!! TEST FILE!!!
*/

#include <Fusion/Fusion.h>

#include <stdio.h>

// TODO: Função requer stack-protect para ABI, implementa sub e add(Existe, ainda implicito) para RSP, pois libc usa SSE!
static void Hello(int valor_jit)
{
    (void)valor_jit;

    puts("Jit chamou isso, ainda fragil!");
}

static const FusHidrNode_t hidr_mov_func = {
    .opcode = HIDR_INSTR_MOV,
    .op_size = HIDR_OP_SIZE_64,
    .mode = HIDR_MODE64,
    .src = {
        .type = HIDR_OPERAND_TYPE_SYM,
        .data.sym.name = "Hello" // RESOLVER EM REALOCAÇÂO! LINKER
    },
    .dst = {
        .type = HIDR_OPERAND_TYPE_REG,
        .data.reg = 1
    }
};
static const FusHidrNode_t hidr_call = {
    .opcode = HIDR_INSTR_CALL,
    .op_size = HIDR_OP_SIZE_64,
    .mode = HIDR_MODE64,
    .src = {
        .type = HIDR_OPERAND_TYPE_REG,
        .data.reg = 1
    },
};
static const FusHidrNode_t mir_exemple2 = {
    .opcode = HIDR_INSTR_RET,
};
static const FusHidrNode_t hidr_addr = {
    .opcode = HIDR_INSTR_ADDR,
    .op_size = HIDR_OP_SIZE_64,
    .mode = HIDR_MODE64,
    .src = {
        .type = HIDR_OPERAND_TYPE_MEM_REF,
        .data.memory_ref = {
            .base = 1,
            .offset = 5
        }
    },
    .dst = {
        .type = HIDR_OPERAND_TYPE_REG,
        .data.reg = 1
    }
};

static FusHidrNode_t hidr_arry[] = {
    [0]=hidr_mov_func,
    [1]=hidr_call,
    [2]=mir_exemple2,
    [3]=hidr_addr
};

int main()
{
    FusInstance fus_instance;
    if (FUS_CreateInstance(&fus_instance,NULL) != FUSION_OK) {
        printf("Init Fusion System Erro!\n");
        return 1;
    }

    FusLinkerContext_t* linker = FUS_CreateLinkerContext(&fus_instance);
    if (!linker) {
        printf("Linker Ctx Failed\n");
        FUS_DestroyInstance(&fus_instance);
        return 1;
    }

    printf("Endereço do Hello: %p \n",&Hello);
    FUS_AddSymbolLinker(linker,"Hello",(uintptr_t)&Hello);

    FusModuleBackend_t x86;
    FUS_LoaderBackend(&fus_instance,&x86, "X86_Backend", FUS_BACKEND_TYPE_STATIC);

    FusCommandBackend backend_define_backend = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .pNext = NULL,
        .backend = x86,
    };
    FusCommandHidr backend_define_hidr = {
        .sType = FUS_COMMAND_SEND_HIDR,
        .pNext = (FusCommandRuleBase_t*)&backend_define_backend,
        .hidr_arry = hidr_arry,
        .hidr_count = 4
    };

    FusBackendReturn_t* backend_data = FUS_MountHidrsBytes(&fus_instance, (FusCommandRuleBase_t*)&backend_define_hidr);
    if (!backend_data) {
        printf("Erro ao Copilar\n");

        FUS_DestroyBackend(x86);
        FUS_DestroyLinkerContext(fus_instance,linker);
        FUS_DestroyInstance(&fus_instance);
        return 1;
    }

    FusCommandBackend linker_define_backend = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .pNext = NULL,
        .backend = x86
    };

    if (FUS_LinkerResolver((FusCommandRuleBase_t*)&linker_define_backend,linker,backend_data) != FUSION_OK) {
        printf("Erro ao Linker\n");

        FUS_DestroyCompiler(fus_instance,backend_data);
        FUS_DestroyBackend(x86);
        FUS_DestroyLinkerContext(fus_instance,linker);
        FUS_DestroyInstance(&fus_instance);
        return 1;
    }
    printf("Pipeline Linker\n");

    FUS_DestroyCompiler(fus_instance,backend_data);
    FUS_DestroyBackend(x86);
    FUS_DestroyLinkerContext(fus_instance,linker);
    FUS_DestroyInstance(&fus_instance);

    return 0;
}