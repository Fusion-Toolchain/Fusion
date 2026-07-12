// META: Fazer funçõe que seja passadas ao backend!
#include <Fusion/FusionTypes.h>
#include "Internal/Memory/Fus_Arena.h"
#include <Internal/Fus_Instance.h>
#include <Internal/Backend/Fus_Backend.h>
#include <Internal/Memory/Fus_Slab.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t from_slab;
} __attribute__((aligned(16))) FusAllocTag_t;

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
static FusBackendGenereteDataBlock_t* FUSI_CreateGenereteDataBlock(FusBackendApi_t* api, size_t need_realoc, size_t buffer_size)
{
    if (unlikely(!api || need_realoc == 0 || buffer_size == 0)) return NULL;

    FusBackendGenereteDataBlock_t* block = api->FusAlloc(api,sizeof(FusBackendGenereteDataBlock_t));
    if (unlikely(!block)) return NULL;

    FusBackendReallocNeed_t* reallocs = api->FusAlloc(api,sizeof(FusBackendReallocNeed_t) * need_realoc);
    if (unlikely(!reallocs)) {
        api->FusFree(api,block);
        return NULL;
    }
    uint8_t* buffer = api->FusAlloc(api,buffer_size);
    if (unlikely(!buffer)) {
        api->FusFree(api,block);
        api->FusFree(api,reallocs);
        return NULL;
    }
    FusMemoryArena_t* arena = FUSI_CreateArena(1*1024);
    if (!arena) {
        api->FusFree(api,block);
        api->FusFree(api,reallocs);
        api->FusFree(api,buffer);
        return NULL;
    }

    block->realoc_capacity = need_realoc;
    block->realoc_count = 0;
    block->realoc = reallocs;

    block->buffer_slab = buffer;
    block->arena = arena;
    block->flag = FUSION_ERRO;

    block->slab_size = buffer_size;
    block->slab_offset = 0;

    block->api = api;
    return block;
}
static void FUSI_DestroyGenereteDataBlock(FusBackendApi_t* api, FusBackendGenereteDataBlock_t* block)
{
    if (unlikely(!api || !block)) return;

    FUSI_DestroyArena(block->arena);
    api->FusFree(api,block->buffer_slab);
    api->FusFree(api,block->realoc);
    api->FusFree(api,block);

}


static FusBackendApi_t interface_api = {
    .FusAlloc = FUSI_BackendHookMalloc,
    .FusFree = FUSI_BackendHookFree,
    .FusCreateTrasfer = FUSI_CreateTransferLifetime,
    .FusDestroyTrasfer = FUSI_DestroyTransferLifetime,
    .FusCreateDataBlock = FUSI_CreateGenereteDataBlock,
    .FusDestroyDataBlock = FUSI_DestroyGenereteDataBlock
};
FusBackendApi_t FUSI_InterfaceDefine()
{
    return interface_api;
}