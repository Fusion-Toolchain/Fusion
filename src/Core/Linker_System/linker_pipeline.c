#include <Internal/Backend/Fus_Backend.h>
#include <Internal/Linker/Fus_Linker.h>
#include <Internal/Linker/Fus_Hashtable.h>

#include <Internal/Fus_TraceTree.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Instance.h>

#include <Fusion/FusionRule.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static inline bool LinkerGetCommandsRequire(
    FusCommandRuleBase_t*  compiler_rule,
    FusModuleBackend** backend
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

static inline void LinkerBackendRealloc(FusModuleBackend* backend, FusBackendRelocContext_t* context_realoc ,FusBackendRelocationOpaqueType_t type)
{
    if (unlikely(!backend)) return;

    struct FusModuleBackend_T* backend_real = *backend;
    backend_real->interface->FUSI_BackendLinkerRelocation(type,context_realoc);
}
static inline bool LinkerCodeResolver(FusLinkerContext linker, FusModuleBackend* backend, FusBackendReturn backend_data)
{
    FusTraceTree trace = NULL;
    FUSIH_INSTANCE_GET_TRACE(&linker->inst_ref,&trace);

    FusBackendTransferLifetime_t*   data  = backend_data->transfer_data;
    FusBackendGenerateDataBlock_t* block = (FusBackendGenerateDataBlock_t*)data->data;

    for (size_t i = 0; i < block->reloc_count; i++) {
        FusBackendRelocationNeed_t*  realoc_backend = &block->reloc[i];
        FusLinkerContextSymbol_t* symbol = FUS_GetSymbolLinker(linker, realoc_backend->name);
        if (unlikely(!symbol)) {
            FUS_PUSH_ERR(trace,FUSION_ERRO,"Linker Resolver Relocation Step-Failed");
            return false;
        }

        FusBackendRelocContext_t context = {
            .buffer     = block->buffer_slab,
            .offset     = realoc_backend->offset,
            .sym_addr   = symbol->local.addr,
            .patch_addr = (uintptr_t)(block->buffer_slab + realoc_backend->offset)
        };
        LinkerBackendRealloc(backend, &context, realoc_backend->type);
    }

    FUS_PUSH_ERR(trace,FUSION_OK,"Linker Resolver Relocation Step-Success");
    return true;
}


FusStatusFlag_t FUS_LinkerResolver(FusCommandRuleBase_t* compiler_rule, FusLinkerContext linker_ctx, FusBackendReturn backend_data)
{
    if (unlikely(!compiler_rule || !backend_data || !linker_ctx)) return FUSION_ERRO;
    FusModuleBackend* backend = NULL;

    if (unlikely(!LinkerGetCommandsRequire(compiler_rule,&backend))) { // REQUIRE CHAIN ARGUMENTS!
        return FUSION_ERRO;
    }
    if (unlikely(!backend)) return FUSION_ERRO;

    if(unlikely(!LinkerCodeResolver(linker_ctx,backend,backend_data))) { // PROCESS!
        return FUSION_ERRO;
    }

    return FUSION_OK;
}