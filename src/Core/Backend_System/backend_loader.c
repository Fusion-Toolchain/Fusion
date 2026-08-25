#include <Internal/Backend/Fus_Backend.h>
#include <Fusion/Backend/FusionBackend.h>
#include <Internal/Backend/Fus_StaticBackend.h>

#include <Internal/Fus_Instance.h>
#include <Internal/Memory/Fus_Slab.h>
#include <Internal/Memory/Fus_LargerBlocks.h>

// LOCAL
#include "backend_internal.h"

// HELPERS
#include <Internal/Helpers/Fus_Helper_Instance.h>
#include <Internal/Helpers/Fus_Helper_Backend.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <Fusion/FusionTypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline FusBackendInterface_t* LoaderStaticInterfaceMethod(const char* name)
{
    ModuleStaticEntry_t* entry = fusiGetStaticBackend(name);
    if (unlikely(!entry)) return NULL;

    FusBackendInterface_t* interface = entry->fn(); // RETURN STATIC TABLE FOR MODULE!!!
    if (unlikely(!interface)) return NULL;

    return interface;
}
static inline FusBackendInterface_t* LoaderBackendInterfaceType(const char* name,FusModuleBackendType_t type)
{
    switch (type) {
        case FUS_BACKEND_TYPE_STATIC: return LoaderStaticInterfaceMethod(name);
        default: return NULL;
    }
}
static inline void BackendDefineInterface(FusInstance instance,FusBackendApi_t* api)
{
    if (unlikely(!instance || !api)) return;
    *api = fusiInterfaceDefine(); // Backend Interface Define
    api->Instance = instance; // REFERENCIA PARA BACKEND, exige variavel que tenha ciclo de vida alto.
}


FusStatusFlag_t fusLoaderBackend(FusInstance instance, FusModuleBackend* ctx,const char* name, FusModuleBackendType_t type)
{
    if (unlikely(!instance || !ctx || !name || type == FUS_BACKEND_TYPE_NONE)) return FUSION_ERRO;
    *ctx = NULL;

    FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(instance);
    if (unlikely(!alloc)) return FUSION_ERRO;

    struct FusModuleBackend_T* ctx_real = FUSIH_ALLOC(alloc, sizeof(struct FusModuleBackend_T));
    if (unlikely(!ctx_real)) return FUSION_ERRO;

    FusBackendApi_t* api = FUSIH_ALLOC(alloc, sizeof(FusBackendApi_t));
    if (unlikely(!api)) {
        FUSIH_FREE(alloc, ctx_real);
        return FUSION_ERRO;
    }

    size_t len = strlen(name) + 1;
    char* name_copy = FUSIH_ALLOC(alloc, len);
    if (unlikely(!name_copy)) {
        FUSIH_FREE(alloc, api);
        FUSIH_FREE(alloc, ctx_real);
        return FUSION_ERRO;
    }
    memcpy(name_copy, name, len);

    BackendDefineInterface(instance,api); // DEFINE API INTERFACE

    FusBackendInterface_t* interface = LoaderBackendInterfaceType(name, type);
    if (unlikely(!interface)) {
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

void fusDestroyBackend(FusModuleBackend backend)
{
    if (unlikely(!backend)) return;
    struct FusModuleBackend_T* backend_real = backend;

    FusInstance instance              = FUSIH_BACKEND_GET_API(&backend)->Instance;
    FusInstanceMyAllocation_t* alloc   = FUSIH_INSTANCE_GET_ALLOC(instance);

    FUSIH_FREE(alloc, (void*)backend_real->name);
    FUSIH_FREE(alloc, backend_real->api);
    FUSIH_FREE(alloc, backend_real);

    backend = NULL;
}