/*
 *  !!! TEST FILE!!!
*/

#include "Fusion/IRTypes/HidrType.h"
#include <Fusion/Fusion.h>

#include <stdio.h>

// TODO: Função requer stack-protect para ABI, implementa sub e add(Existe, ainda implicito) para RSP, pois libc usa SSE!
static void Hello(int valor_jit)
{
   puts("Jit chamou isso, ainda fragil!");
}

static const FusHidrNode_t hidr_mov_func = {
    .opcode = HIDR_INSTR_MOV,
    .op_size = HIDR_OP_SIZE_64,
    .mode = HIDR_MODE64,
    .src = {
        .type = HIDR_OPERAND_TYPE_SYM,
        .data.sym.name = "Hello"
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
    FusBufferContext_t* buffer = FUS_CreateBufferCode(1024);
    FusLinkerContext_t* linker = FUS_CreateLinkerContext();

    printf("Endereço do Hello: %p \n",&Hello);
    FUS_AddSymbolLinker(linker,"Hello",(uintptr_t)&Hello);

    FusTracedErro_t* erro = FUS_CreateTracedErro();
    FusModuleBackend_t* x86 = FUS_LoaderBackend("X86_Backend", FUS_BACKEND_TYPE_STATIC);

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
    FusCommandTraceContext backend_define_trace = {
        .sType = FUS_COMMAND_SEND_TRACE,
        .pNext = (FusCommandRuleBase_t*)&backend_define_hidr,
        .trace_data = erro
    };

    FusBackendReturn_t* backend_data = FUS_MountHidrsBytes((FusCommandRuleBase_t*)&backend_define_trace);
    if (!backend_data) {
        printf("Erro ao Copilar: %s\n", FUS_GetTracedErroMsg(erro));
        FUS_DestroyBufferCode(buffer);
        FUS_DestroyTracedErro(erro);
        FUS_DestroyBackend(x86);
        FUS_DestroyLinkerContext(linker);
        return 1;
    }

    FusCommandTraceContext linker_define_trace = {
        .sType = FUS_COMMAND_SEND_TRACE,
        .pNext = NULL,
        .trace_data = erro
    };
    FusCommandBackend linker_define_backend = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .pNext = (FusCommandRuleBase_t*)&linker_define_trace,
        .backend = x86
    };

    if (FUS_LinkerResolver((FusCommandRuleBase_t*)&linker_define_backend,linker,backend_data) != FUSION_OK) {
        printf("Erro ao Linker: %s\n",FUS_GetTracedErroMsg(erro));
        FUS_DestroyBufferCode(buffer);
        FUS_DestroyTracedErro(erro);
        FUS_DestroyBackend(x86);
        FUS_DestroyLinkerContext(linker);
        return 1;
    }

    FUS_DestroyBufferCode(buffer);
    FUS_DestroyTracedErro(erro);
    FUS_DestroyBackend(x86);
    FUS_DestroyLinkerContext(linker);

    return 0;
}