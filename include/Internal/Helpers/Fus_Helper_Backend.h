#ifndef FUSION_INTERNAL_HELPER_BACKEND_H
#define FUSION_INTERNAL_HELPER_BACKEND_H
#include <Internal/Backend/Fus_Backend.h>

// HELPER AGENT
#include "Fus_Helper_Codebase.h"

// ─── Backend Helpers ────────────────────────────────────────────
static inline FusBackendApi_t* FUSIH_BACKEND_GET_API(FusModuleBackend* backend)
{
    if (unlikely(!backend)) return NULL;
    struct FusModuleBackend_T* real = *backend;
    if (unlikely(!real)) return NULL;
    return real->api;
}
static inline FusBackendInterface_t* FUSIH_BACKEND_GET_INTERFACE(FusModuleBackend* backend)
{
    if (unlikely(!backend)) return NULL;
    struct FusModuleBackend_T* real = *backend;

    if (unlikely(!real)) return NULL;
    return real->interface;
}
static inline const char* FUSIH_BACKEND_GET_NAME(FusModuleBackend* backend)
{
    if (unlikely(!backend)) return NULL;
    struct FusModuleBackend_T* real = *backend;
    if (unlikely(!real)) return NULL;
    return real->name;
}

// ─── Api Helpers ────────────────────────────────────────────────

static inline void* FUSIH_API_ALLOC(FusBackendApi_t* api, size_t size)
{
    if (unlikely(!api)) return NULL;
    return api->FusAlloc(api, size);
}
static inline void FUSIH_API_FREE(FusBackendApi_t* api, void* ptr)
{
    if (unlikely(!api || !ptr)) return;
    api->FusFree(api, ptr);
}
static inline FusBackendTrasferLifeTime_t* FUSIH_API_CREATE_TRANSFER_LIFETIME(FusBackendApi_t* api, void* data, void (*free)(const void* data))
{
    if (unlikely(!api || !data)) return NULL;
    return api->FusCreateTrasfer(api,data,free);
}
static inline void FUSIH_API_DESTROY_TRANSFER_LIFETIME(FusBackendApi_t* api, FusBackendTrasferLifeTime_t* data_tranfer)
{
    if (unlikely(!api || !data_tranfer)) return;
    api->FusDestroyTrasfer(api,data_tranfer);
}

#endif