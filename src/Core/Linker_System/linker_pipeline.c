#include <Internal/Fus_Error.h>
#include <Internal/Linker/Fus_Linker.h>
#include <Internal/Linker/Fus_Hashtable.h>
#include <Internal/Fus_Backend.h>

#include <Fusion/FusionRule.h>
#include <Fusion/FusionErro.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static inline bool _LinkerGetCommandsRequire(
    FusCommandRuleBase_t*  compiler_rule,
    FusTracedErro_t** trace,
    FusModuleBackend_t** backend
)
{
   if (!compiler_rule) return false;

   FusCommandRuleBase_t* node = compiler_rule;
   while (node) {
        switch (node->sType) {
            case FUS_COMMAND_SEND_BACKEND: {
                *backend = ((FusCommandBackend*)node)->backend;
                break;
            }
            case FUS_COMMAND_SEND_TRACE: {
                *trace = ((FusCommandTraceContext*)node)->trace_data;
                break;
            }
            default: return false;
        }
        node = (FusCommandRuleBase_t*)node->pNext;
   } 
   return true;
}

static void LinkerClearBackendReturn(FusModuleBackend_t* backend, FusBackendReturn_t* backend_data)
{
    if (!backend || !backend_data) return;
    FusBackendApi_t* api = backend->api;
    FusBackendTrasferLifeTime_t* data = backend_data->transfer_data;

    data->free(data->data);
    api->FusFree(data);
}

static inline void LinkerBackendRealloc(FusModuleBackend_t* backend, FusTracedErro_t* trace, FusBackendRelocContext_t* context_realoc ,FusBackendRealocOpaqueType_t type)
{
    if (!backend) return;
    backend->interface->FUSI_BackendLinkerRealloc(trace,type,context_realoc);
}
static inline bool LinkerCodeResolver(FusLinkerContext_t* linker,FusTracedErro_t* trace,FusModuleBackend_t* backend, FusBackendReturn_t* backend_data)
{
    FusBackendTrasferLifeTime_t* data = backend_data->transfer_data;

    FusBackendGenereteDataBlock_t* block = (FusBackendGenereteDataBlock_t*)data->data;
    for (size_t i = 0; i < block->realoc_count; i++) {
        FusBackendReallocNeed_t* realoc_backend = &block->realoc[i];
        FusLinkerContextSymbol_t* symbol = FUS_GetSymbolLinker(linker,realoc_backend->name);
        if (!symbol) {
            trace->msg = "Linker: Symbol not resolver!";
            trace->type = FUS_TRACED_TYPE_ERRO;
            trace->local = FUS_TRACED_LOCAL_LINKER;

            return false;
        }

        FusBackendRelocContext_t context = {
            .buffer = block->buffer,
            .offset = realoc_backend->offset,
            .sym_addr = symbol->local.addr,
            .patch_addr = (uintptr_t)(block->buffer->buffer + realoc_backend->offset) // RESOLVIDO APOS A MONTAGEM
        };
        LinkerBackendRealloc(backend,trace,&context,realoc_backend->type);
    }

    for (size_t i = 0; i < block->buffer->offset; i++) {
        printf(" %02X",block->buffer->buffer[i]);
    }
    printf("\n");

    LinkerClearBackendReturn(backend,backend_data);
    return true;
}
FusStatusFlag_t FUS_LinkerResolver(FusCommandRuleBase_t* compiler_rule, FusLinkerContext_t* linker_ctx, FusBackendReturn_t* backend_data)
{
    if (!compiler_rule || !backend_data || !linker_ctx) return FUSION_ERRO;

    FusTracedErro_t* trace = NULL;
    FusModuleBackend_t* backend = NULL;
    if (!_LinkerGetCommandsRequire(compiler_rule,&trace,&backend)) return FUSION_ERRO;
    if (!trace || !backend) return FUSION_ERRO;

    if(!LinkerCodeResolver(linker_ctx,trace,backend,backend_data)) return FUSION_ERRO;

    return FUSION_OK;
}