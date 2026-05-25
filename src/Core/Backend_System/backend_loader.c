// HELPERS
#include <Internal/Helpers/Fus_Helper_Instance.h>
#include <Internal/Helpers/Fus_Helper_Backend.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>

#include <Internal/Fus_Backend.h>
#include <Internal/Fus_StaticBackend.h>

#include <Internal/Fus_Instance.h>
#include <Internal/Memory/Fus_Slab.h>
#include <Internal/Memory/Fus_LargerBlocks.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline FusBackendInterface_t* LoaderStaticInterfaceMethod(FusBackendApi_t* api,const char* name)
{
    ModuleStaticEntry_t* entry = FUS_GetStaticBackend(name);
    if (!entry) return NULL;

    FusBackendInterface_t* interface = entry->fn(api); // RETURN STATIC TABLE FOR MODULE!!!
    if (!interface) return NULL;

    return interface;
}
static inline FusBackendInterface_t* LoaderBackendInterfaceType(FusBackendApi_t* api,const char* name,FusModuleBackendType_t type)
{
    switch (type) {
        case FUS_BACKEND_TYPE_STATIC: return LoaderStaticInterfaceMethod(api,name);
        default: return NULL;
    }
}

typedef struct {
    uint8_t from_slab;
} FusAllocTag_t;

void* FUSI_BackendHookMalloc(FusBackendApi_t* api, size_t size)
{
    if (!api || size == 0) return NULL;

    struct FusInstance_T* instance = *api->Instance;
    size_t total = size + sizeof(FusAllocTag_t);
    FusAllocTag_t* tag;

    if (size <= 4096) {
        tag = FUSI_AllocSlab(instance->slab, total);
        if (!tag) return NULL;
        tag->from_slab = 1;
    } else {
        tag = FUSI_AllocLargerBlocks(&instance->larger_alloc, total);
        if (!tag) return NULL;
        tag->from_slab = 0;
    }

    return (void*)(tag + 1);
}

void FUSI_BackendHookFree(FusBackendApi_t* api, void* ptr)
{
    if (!api || !ptr) return;

    struct FusInstance_T* instance = *api->Instance;
    FusAllocTag_t* tag = (FusAllocTag_t*)ptr - 1;

    if (tag->from_slab)
        FUSI_FreeSlab(instance->slab, tag);
    else
        FUSI_FreeLargerBlocks(&instance->larger_alloc, tag);
}

FusStatusFlag_t FUS_LoaderBackend(FusInstance* instance, FusModuleBackend_t* ctx,const char* name, FusModuleBackendType_t type)
{
    if (!instance || !ctx || !name || type == FUS_BACKEND_TYPE_NONE) return FUSION_ERRO;
    *ctx = NULL;

    FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(instance);
    if (!alloc) return FUSION_ERRO;

    struct FusModuleBackend_T* ctx_real = FUSIH_ALLOC(alloc, sizeof(struct FusModuleBackend_T));
    if (!ctx_real) return FUSION_ERRO;

    FusBackendApi_t* api = FUSIH_ALLOC(alloc, sizeof(FusBackendApi_t));
    if (!api) {
        FUSIH_FREE(alloc, ctx_real);
        return FUSION_ERRO;
    }

    size_t len = strlen(name) + 1;
    char* name_copy = FUSIH_ALLOC(alloc, len);
    if (!name_copy) {
        FUSIH_FREE(alloc, api);
        FUSIH_FREE(alloc, ctx_real);
        return FUSION_ERRO;
    }
    memcpy(name_copy, name, len);

    api->FusAlloc = FUSI_BackendHookMalloc;
    api->FusFree  = FUSI_BackendHookFree;
    api->Instance = instance;

    FusBackendInterface_t* interface = LoaderBackendInterfaceType(api, name, type);
    if (!interface) {
        FUSIH_FREE(alloc, (void*)name_copy);
        FUSIH_FREE(alloc, api);
        FUSIH_FREE(alloc, ctx_real);
        return FUSION_ERRO;
    }

    ctx_real->name      = name_copy;
    ctx_real->interface = interface;
    ctx_real->type      = type;
    ctx_real->api       = api;

    *ctx = ctx_real;
    return FUSION_OK;
}

void FUS_DestroyBackend(FusModuleBackend_t backend)
{
    if (!backend) return;
    struct FusModuleBackend_T* backend_real = backend;

    FusInstance* instance              = FUSIH_BACKEND_GET_API(&backend)->Instance;
    FusInstanceMyAllocation_t* alloc   = FUSIH_INSTANCE_GET_ALLOC(instance);

    FUSIH_FREE(alloc, (void*)backend_real->name);
    FUSIH_FREE(alloc, backend_real->api);
    FUSIH_FREE(alloc, backend_real);

    backend = NULL;
}