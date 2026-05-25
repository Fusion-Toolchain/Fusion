#ifndef FUSION_INTERNAL_HELPER_BACKEND_H
#define FUSION_INTERNAL_HELPER_BACKEND_H
#include <Internal/Fus_Backend.h>

// ─── Backend Helpers ────────────────────────────────────────────
static inline FusBackendApi_t* FUSIH_BACKEND_GET_API(FusModuleBackend_t* backend)
{
    if (!backend) return NULL;
    struct FusModuleBackend_T* real = *backend;
    if (!real) return NULL;
    return real->api;
}
static inline FusBackendInterface_t* FUSIH_BACKEND_GET_INTERFACE(FusModuleBackend_t* backend)
{
    if (!backend) return NULL;
    struct FusModuleBackend_T* real = *backend;
    if (!real) return NULL;
    return real->interface;
}
static inline const char* FUSIH_BACKEND_GET_NAME(FusModuleBackend_t* backend)
{
    if (!backend) return NULL;
    struct FusModuleBackend_T* real = *backend;
    if (!real) return NULL;
    return real->name;
}

// ─── Api Helpers ────────────────────────────────────────────────

static inline void* FUSIH_API_ALLOC(FusBackendApi_t* api, size_t size)
{
    if (!api) return NULL;
    return api->FusAlloc(api, size);
}
static inline void FUSIH_API_FREE(FusBackendApi_t* api, void* ptr)
{
    if (!api || !ptr) return;
    api->FusFree(api, ptr);
}

#endif