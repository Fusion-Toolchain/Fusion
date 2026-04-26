#include <Internal/Fus_Backend.h>
#include <Internal/Fus_StaticBackend.h>

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

static void* _Hook_Malloc(size_t size)
{
    if (size == 0) return NULL;
    printf("HOOK FUSION -> Backend Alocou: %ld\n",size);

    return malloc(size);
}
static void _Hook_Free(void* ptr)
{
    if (!ptr) return;
    printf("HOOK FUSION -> Backend Liberou: %p\n",ptr);

    free(ptr);
}

FusModuleBackend_t* FUS_LoaderBackend(const char* name, FusModuleBackendType_t type)
{
    if (!name || type == FUS_BACKEND_TYPE_NONE) return NULL;

    FusModuleBackend_t* ctx = malloc(sizeof(FusModuleBackend_t));
    if (!ctx) return NULL;
    FusBackendApi_t* api = malloc(sizeof(FusBackendApi_t));
    if (!api) {
        free(ctx);
        return NULL;
    }
    api->FusAlloc = _Hook_Malloc;
    api->FusFree = _Hook_Free;

    FusBackendInterface_t* interface = LoaderBackendInterfaceType(api,name,type);
    if (!interface) {
        free(ctx);
        return NULL;
    }

    ctx->name = strdup(name);
    ctx->interface = interface;
    ctx->type = type;
    ctx->api = api;

    return ctx;
}

void FUS_DestroyBackend(FusModuleBackend_t* backend)
{
    if (!backend) return;

    free((void*)backend->name);
    free(backend->api);
    free(backend);
}