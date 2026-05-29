#include "Fusion/Backend/FusionBackend.h"
#include "Fusion/FusionTypes.h"
#include <Fusion/Fusion.h>

#include <Internal/Fus_Backend.h>
#include <Internal/Fus_Instance.h>

// HELPERS
#include <Internal/Helpers/Fus_Helper_Instance.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Backend.h>

// TYPES
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

static inline bool _ArgumentProcessMountHidrBytes(
    FusCommandRuleBase_t*  compiler_rule,
    struct FusModuleBackend_T** backend,
    FusHidrNode_t**        hidr,
    size_t*                count
)
{
    if (unlikely(!compiler_rule)) return false;

    FusCommandRuleBase_t* node = compiler_rule;
    while (node) {
        switch (node->sType) {
            case FUS_COMMAND_SEND_BACKEND: {
                FusCommandBackend* b = (FusCommandBackend*)node;
                *backend = (struct FusModuleBackend_T*)b->backend;
                break;
            }
            case FUS_COMMAND_SEND_HIDR: {
                FusCommandHidr* r = (FusCommandHidr*)node;
                *hidr  = r->hidr_arry;
                *count = r->hidr_count;
                break;
            }
            default: break;
        }
        node = (FusCommandRuleBase_t*)node->pNext;
    }

    return true;
}

FusBackendReturn_t* FUS_MountHidrsBytes(FusInstance* instance,FusCommandRuleBase_t* compiler_rule)
{
    if (unlikely(!instance)) return NULL;
    if (unlikely(!compiler_rule)) return NULL;
    //FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(instance);

    FusBackendReturn_t* ctx = FUSIH_INSTANCE_ALLOC(instance, sizeof(FusBackendReturn_t));
    if (unlikely(!ctx)) {
        // LOG AQUI
        goto error;
    }

    struct FusModuleBackend_T* backend = NULL;
    FusHidrNode_t* hidr = NULL;
    size_t count = 0;

    if(!_ArgumentProcessMountHidrBytes(compiler_rule,&backend,&hidr,&count)) goto error;
    if (unlikely(!backend || !hidr || !count)) goto error;
    if (unlikely(count == 0)) {
        // LOG AQUI
        goto error;
    }

    FusBackendInterface_t* interface = backend->interface;
    FusBackendTrasferLifeTime_t* backend_data = interface->FUSI_BackendMountHidrArry(hidr,count);
    if (unlikely(!backend_data)) {
        // LOG AQUI
        goto error;
    }

    ctx->transfer_data = backend_data;
    ctx->api = backend->api;

    return ctx; // RETURN SUCCESS

    error:
    if (ctx) {
        FUSIH_API_DESTROY_TRANSFER_LIFETIME(ctx->api,ctx->transfer_data);
        FUSIH_INSTANCE_FREE(instance,ctx);
    }
    return NULL; // RETURN ERROR
}

FusBufferContext_t* FUS_GetStreamBufferCompiler(FusBackendReturn_t* ctx_backend)
{
    if (unlikely(!ctx_backend)) return NULL;

    FusBackendGenereteDataBlock_t* data_block = (FusBackendGenereteDataBlock_t*)ctx_backend->transfer_data->data;

    return data_block->buffer;
}

void FUS_DestroyCompiler(FusInstance instance, FusBackendReturn_t* ctx_backend)
{
    if (unlikely(!instance || !ctx_backend)) return;
    //FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(&instance);

    FUSIH_API_DESTROY_TRANSFER_LIFETIME(ctx_backend->api,ctx_backend->transfer_data);
    FUSIH_INSTANCE_FREE(&instance,ctx_backend);
}