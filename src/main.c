/*
 *  !!! TEST FILE!!!
*/

#include "Fusion/FusionTypes.h"
#include <Fusion/Fusion.h>

#include <bits/time.h>
#include <stdio.h>
#include <time.h>

// TODO: Função requer stack-protect para ABI, implementa sub e add(Existe, ainda implicito) para RSP, pois libc usa SSE!
static void Hello(int valor_jit)
{
    (void)valor_jit;

    puts("Jit chamou isso, ainda fragil!");
}

// 1. Bloco Abstrato para MOV (Símbolo para Registrador)
FusHidrNode_t NewMovSym(const char* sym_name, int dst_reg) {
    return (FusHidrNode_t){
        .opcode = HIDR_INSTR_MOV,
        .op_size = HIDR_OP_SIZE_64,
        .mode = HIDR_MODE64,
        .src = { .type = HIDR_OPERAND_TYPE_SYM, .data.sym.name = sym_name },
        .dst = { .type = HIDR_OPERAND_TYPE_REG, .data.reg = dst_reg }
    };
}

// 2. Bloco Abstrato para CALL (Via Registrador)
FusHidrNode_t NewCallReg(int src_reg) {
    return (FusHidrNode_t){
        .opcode = HIDR_INSTR_CALL,
        .op_size = HIDR_OP_SIZE_64,
        .mode = HIDR_MODE64,
        .src = { .type = HIDR_OPERAND_TYPE_REG, .data.reg = src_reg }
    };
}

// 3. Bloco Abstrato para RET
FusHidrNode_t NewRet(void) {
    return (FusHidrNode_t){
        .opcode = HIDR_INSTR_RET
    };
}

// 4. Bloco Abstrato para ADDR (Referência de Memória para Registrador)
FusHidrNode_t NewAddrMem(int base_reg, int offset, int dst_reg) {
    return (FusHidrNode_t){
        .opcode = HIDR_INSTR_ADDR,
        .op_size = HIDR_OP_SIZE_64,
        .mode = HIDR_MODE64,
        .src = {
            .type = HIDR_OPERAND_TYPE_MEM_REF,
            .data.memory_ref = { .base = base_reg, .offset = offset }
        },
        .dst = { .type = HIDR_OPERAND_TYPE_REG, .data.reg = dst_reg }
    };
}

int main()
{
    FusHidrNode_t hidr_arry[] = {
        [0] = NewMovSym("Hello", 1),  // Move o símbolo "Hello" para o Reg 1
        [1] = NewCallReg(1),          // Chama a função apontada pelo Reg 1
        [2] = NewRet(),               // Retorna
    };

    FusInstance fus_instance;
    if (FUS_CreateInstance(&fus_instance,NULL) != FUSION_OK) {
        printf("Init Fusion System Erro!\n");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    FusLinkerContext_t* linker = FUS_CreateLinkerContext(&fus_instance);
    if (!linker) {
        printf("Linker Ctx Failed\n");
        FUS_DestroyInstance(&fus_instance);
        return 1;
    }

    //printf("Endereço do Hello: %p \n",&Hello);
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

    clock_gettime(CLOCK_MONOTONIC,&end);

    FusBufferContext_t* buffer_gen = FUS_GetStreamBufferCompiler(backend_data);
    if (!buffer_gen) {
        printf("Nenhum buffer attatch!\n");
        goto err;
    }

    FUS_ExecutableBuffer(buffer_gen);
    void(*run)(void) = (void(*)(void))buffer_gen->buffer;
    run(); // RUN

err:

    FUS_DestroyCompiler(fus_instance,backend_data);
    FUS_DestroyBackend(x86);
    FUS_DestroyLinkerContext(fus_instance,linker);
    FUS_DestroyInstance(&fus_instance);

    long long nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    double microseconds = (double)nanoseconds / 1000.0;
    printf("\n Execute Time: %lld ns (%.3f us)\n", nanoseconds, microseconds);

    return 0;
}
