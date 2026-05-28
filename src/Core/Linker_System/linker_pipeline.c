#include <Internal/Linker/Fus_Linker.h>
#include <Internal/Linker/Fus_Hashtable.h>
#include <Internal/Fus_Backend.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

#include <Fusion/FusionRule.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static inline bool LinkerGetCommandsRequire(
    FusCommandRuleBase_t*  compiler_rule,
    FusModuleBackend_t** backend
)
{
   if (unlikely(!compiler_rule)) return false;

   FusCommandRuleBase_t* node = compiler_rule;
   while (node) {
        switch (node->sType) {
            case FUS_COMMAND_SEND_BACKEND: {
                *backend = &((FusCommandBackend*)node)->backend;
                break;
            }
            default: break;
        }
        node = (FusCommandRuleBase_t*)node->pNext;
   } 
   return (*backend);
}

static inline void LinkerBackendRealloc(FusModuleBackend_t* backend, FusBackendRelocContext_t* context_realoc ,FusBackendRealocOpaqueType_t type)
{
    if (unlikely(!backend)) return;

    struct FusModuleBackend_T* backend_real = *backend;
    backend_real->interface->FUSI_BackendLinkerRealloc(type,context_realoc);
}
static inline bool LinkerCodeResolver(FusLinkerContext_t* linker,FusModuleBackend_t* backend, FusBackendReturn_t* backend_data)
{
    FusBackendTrasferLifeTime_t* data = backend_data->transfer_data;

    FusBackendGenereteDataBlock_t* block = (FusBackendGenereteDataBlock_t*)data->data;
    for (size_t i = 0; i < block->realoc_count; i++) {
        FusBackendReallocNeed_t* realoc_backend = &block->realoc[i];

        FusLinkerContextSymbol_t* symbol = FUS_GetSymbolLinker(linker,realoc_backend->name);
        if (unlikely(!symbol)) { // NOT FOUND SYMBOL
            return false;
        }

        FusBackendRelocContext_t context = {
            .buffer = block->buffer,
            .offset = realoc_backend->offset,
            .sym_addr = symbol->local.addr,
            .patch_addr = (uintptr_t)(block->buffer->buffer + realoc_backend->offset) // RESOLVIDO APOS A MONTAGEM
        };
        LinkerBackendRealloc(backend,&context,realoc_backend->type);
    }

    // DEBUG, REMOVE PLS
    for (size_t i = 0; i < block->buffer->offset; i++) {
        printf(" %02X",block->buffer->buffer[i]);
    }
    printf("\n");

    return true;
}
FusStatusFlag_t FUS_LinkerResolver(FusCommandRuleBase_t* compiler_rule, FusLinkerContext_t* linker_ctx, FusBackendReturn_t* backend_data)
{
    if (unlikely(!compiler_rule || !backend_data || !linker_ctx)) return FUSION_ERRO;
    FusModuleBackend_t* backend = NULL;

    if (unlikely(!LinkerGetCommandsRequire(compiler_rule,&backend))) { // REQUIRE CHAIN ARGUMENTS!
        return FUSION_ERRO;
    }

    if (unlikely(!backend)) return FUSION_ERRO;

    if(unlikely(!LinkerCodeResolver(linker_ctx,backend,backend_data))) { // PROCESS!
        return FUSION_ERRO;
    }

    return FUSION_OK;
}