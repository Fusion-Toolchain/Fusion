// META: Fazer funçõe que seja passadas ao backend!
#include <Internal/Fus_Instance.h>
#include <Internal/Fus_Backend.h>
#include <Internal/Memory/Fus_Slab.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stdlib.h>

typedef struct {
    uint8_t from_slab;
} FusAllocTag_t;

static void* FUSI_BackendHookMalloc(FusBackendApi_t* api, size_t size)
{
    if (unlikely(!api || size == 0)) return NULL;

    struct FusInstance_T* instance = *api->Instance;
    size_t total = size + sizeof(FusAllocTag_t);
    FusAllocTag_t* tag;

    if (size <= 4096) { // SLAB
        tag = FUSI_AllocSlab(instance->slab, total);
        if (!tag) return NULL;
        tag->from_slab = 1;
    } else { // LARGER BLOCK
        tag = FUSI_AllocLargerBlocks(&instance->larger_alloc, total);
        if (!tag) return NULL;
        tag->from_slab = 0;
    }

    return (void*)(tag + 1);
}
static void FUSI_BackendHookFree(FusBackendApi_t* api, void* ptr)
{
    if (unlikely(!api || !ptr)) return;

    struct FusInstance_T* instance = *api->Instance;
    FusAllocTag_t* tag = (FusAllocTag_t*)ptr - 1;

    if (tag->from_slab) { // SLAB
        FUSI_FreeSlab(instance->slab, tag);
    } else { // LARGER BLOCK
        FUSI_FreeLargerBlocks(&instance->larger_alloc, tag);
    }
}
static FusBackendTrasferLifeTime_t* FUSI_CreateTransferLifetime(FusBackendApi_t* api,void* data,void (*free)(const void*))
{
    if (unlikely(!api || !data)) return NULL;
    struct FusInstance_T* instance = *api->Instance;

    FusBackendTrasferLifeTime_t* trans = FUSI_AllocSlab(instance->slab,sizeof(FusBackendTrasferLifeTime_t));
    if (unlikely(!trans)) return NULL;

    trans->data = data;
    trans->free = free;

    return trans;
}
static void FUSI_DestroyTransferLifetime(FusBackendApi_t* api, FusBackendTrasferLifeTime_t* transfer)
{
    if (unlikely(!api || !transfer)) return;
    struct FusInstance_T* instance = *api->Instance;

    transfer->free(transfer->data);
    FUSI_FreeSlab(instance->slab,transfer);
}

static FusBackendApi_t interface_api = {
    .FusAlloc = FUSI_BackendHookMalloc,
    .FusFree = FUSI_BackendHookFree,
    .FusCreateTrasfer = FUSI_CreateTransferLifetime,
    .FusDestroyTrasfer = FUSI_DestroyTransferLifetime
};
FusBackendApi_t FUSI_InterfaceDefine()
{
    return interface_api;
}