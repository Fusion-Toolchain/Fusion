#include <Internal/Fus_TraceTree.h>
#include <Fusion/Backend/FusionBackend.h>
#include <Fusion/Fusion.h>

#include <Internal/Backend/Fus_Backend.h>
#include <Internal/IRTypes/Fus_CodeBuffer.h>
#include <Internal/Fus_Instance.h>

// HELPERS
#include <Internal/Helpers/Fus_Helper_Instance.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Backend.h>

// TYPES
#include <Fusion/FusionTypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

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
                if (!r->code) return false;
                *hidr  = r->code->code_arry; // PROVISORY SOLUTION
                *count = r->code->code_count;
                break;
            }
            default: break;
        }
        node = (FusCommandRuleBase_t*)node->pNext;
    }

    return true;
}

FusStatusFlag_t FUS_MountHidrsBytes(FusInstance* instance,FusCommandRuleBase_t* compiler_rule,FusBackendReturn* out)
{
    if (unlikely(!instance)) return FUSION_ERRO;
    if (unlikely(!compiler_rule)) return FUSION_ERRO;
    if (unlikely(!out)) return FUSION_ERRO;
    *out = NULL;

    struct FusBackendReturn_T* ctx = FUSIH_INSTANCE_ALLOC(instance, sizeof(struct FusBackendReturn_T));
    if (unlikely(!ctx)) {
        FUS_PUSH_ERR(FUSIH_INSTANCE_GET_TRACE(instance),FUSION_ERRO,
            "Fail alloc Backend Return!");
        goto error;
    }

    struct FusModuleBackend_T* backend = NULL;
    FusHidrNode_t* hidr = NULL;
    size_t count = 0;

    if(!_ArgumentProcessMountHidrBytes(compiler_rule,&backend,&hidr,&count)) goto error;
    if (unlikely(!backend || !hidr || !count)) goto error;
    if (unlikely(count == 0)) {
        FUS_PUSH_ERR(
            FUSIH_INSTANCE_GET_TRACE(instance),FUSION_ERRO,
            "Backend Compiler Argumento Failed!");
        goto error;
    }

    FusBackendInterface_t* interface = backend->interface;
    FusBackendTrasferLifeTime_t* backend_data = interface->FUSI_BackendMountHidrArry(hidr,count);
    if (unlikely(!backend_data)) {
        FUS_PUSH_ERR(
            FUSIH_INSTANCE_GET_TRACE(instance),FUSION_ERRO,
            "Backend fail step mount Bytes, entry backend!");
        goto error;
    }

    ctx->transfer_data = backend_data;
    ctx->api = backend->api;
    *out = ctx;

    FUS_PUSH_ERR(FUSIH_INSTANCE_GET_TRACE(instance),FUSION_OK,
        "Backend Mount Byte-Step OK!");
    return FUSION_OK;

    error:
    if (ctx) {
        FUSIH_API_DESTROY_TRANSFER_LIFETIME(ctx->api,ctx->transfer_data);
        FUSIH_INSTANCE_FREE(instance,ctx);
    }
    return FUSION_ERRO; // RETURN ERROR
}

FusBufferContext_t* FUS_GetStreamBufferCompiler(FusBackendReturn ctx_backend)
{
    if (unlikely(!ctx_backend)) return NULL;

    FusBackendGenereteDataBlock_t* data_block = (FusBackendGenereteDataBlock_t*)ctx_backend->transfer_data->data;
    FusBufferContext_t* exec = FUS_CreateBufferCode(data_block->slab_size);
    if (unlikely(!exec)) return NULL;

    memcpy(exec->buffer, data_block->buffer_slab,data_block->slab_size);
    exec->offset = data_block->slab_offset; // PASS OFFSET

    return exec;
}

void FUS_DestroyCompiler(FusInstance instance, FusBackendReturn ctx_backend)
{
    if (unlikely(!instance || !ctx_backend)) return;

    FUSIH_API_DESTROY_TRANSFER_LIFETIME(ctx_backend->api,ctx_backend->transfer_data);
    FUSIH_INSTANCE_FREE(&instance,ctx_backend);
}