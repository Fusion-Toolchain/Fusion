// HELPERS
#include "Fusion/Backend/FusionBackend.h"
#include <Internal/Helpers/Fus_Helper_Instance.h>

#include <Fusion/Fusion.h>

#include <Internal/Fus_Backend.h>
#include <Internal/Fus_Instance.h>

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
    if (!compiler_rule) return false;

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
    if (!instance) return NULL;
    if (!compiler_rule) return NULL;
    FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(instance);

    FusBackendReturn_t* ctx = FUSIH_INSTANCE_ALLOC(instance, sizeof(FusBackendReturn_t));
    if (!ctx) {
        // LOG AQUI
        return NULL;
    }

    struct FusModuleBackend_T* backend = NULL;
    FusHidrNode_t* hidr = NULL;
    size_t count = 0;

    if(!_ArgumentProcessMountHidrBytes(compiler_rule,&backend,&hidr,&count)) goto error;
    if (!backend || !hidr || !count) goto error;
    if (count == 0) {
        // LOG AQUI
        goto error;
    }

    FusBackendInterface_t* interface = backend->interface;
    FusBackendApi_t* API = backend->api;
    FusBackendTrasferLifeTime_t* backend_data = interface->FUSI_BackendMountHidrArry(hidr,count);
    if (!backend_data) {
        // LOG AQUI
        goto error;
    }

    ctx->transfer_data = backend_data;
    ctx->api = backend->api;

    return ctx; // RETURN SUCCESS

    error:
    if (ctx) {
        if (ctx->transfer_data) FUSIH_INSTANCE_FREE(alloc->userdata,ctx->transfer_data);
        FUSIH_INSTANCE_FREE(alloc->userdata,ctx);
    }
    return NULL; // RETURN ERROR
}

void FUS_DestroyCompiler(FusInstance instance, FusBackendReturn_t* ctx_backend)
{
    if (!instance || !ctx_backend) return;

    FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(&instance);

    if (ctx_backend->transfer_data) {
        if (ctx_backend->transfer_data->data) ctx_backend->transfer_data->free(ctx_backend->transfer_data->data);
        FUSIH_INSTANCE_FREE(alloc->userdata, ctx_backend->transfer_data);
    }

    FUSI_BackendHookFree(ctx_backend->api, ctx_backend->transfer_data);
    FUSIH_INSTANCE_FREE(&instance,ctx_backend);
}