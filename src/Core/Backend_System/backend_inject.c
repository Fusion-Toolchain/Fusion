#include <Internal/Memory/Fus_Arena.h>
#include <Internal/Fus_Instance.h>
#include <Internal/Backend/Fus_Backend.h>
#include <Internal/Memory/Fus_Slab.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <Fusion/FusionTypes.h>
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

    struct FusInstance_T* instance = api->Instance;
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

    struct FusInstance_T* instance = api->Instance;
    FusAllocTag_t* tag = (FusAllocTag_t*)ptr - 1;

    if (tag->from_slab) { // SLAB
        FUSI_FreeSlab(instance->slab, tag);
    } else { // LARGER BLOCK
        FUSI_FreeLargerBlocks(&instance->larger_alloc, tag);
    }
}
static FusBackendTransferLifetime_t* FUSI_CreateTransferLifetime(FusBackendApi_t* api,void* data,void (*free)(const void*))
{
    if (unlikely(!api || !data)) return NULL;
    struct FusInstance_T* instance = api->Instance;

    FusBackendTransferLifetime_t* trans = FUSI_AllocSlab(instance->slab,sizeof(FusBackendTransferLifetime_t));
    if (unlikely(!trans)) return NULL;

    trans->data = data;
    trans->free = free;

    return trans;
}
static void FUSI_DestroyTransferLifetime(FusBackendApi_t* api, FusBackendTransferLifetime_t* transfer)
{
    if (unlikely(!api || !transfer)) return;
    struct FusInstance_T* instance = api->Instance;

    transfer->free(transfer->data);
    FUSI_FreeSlab(instance->slab,transfer);
}
static FusBackendGenerateDataBlock_t* FUSI_CreateDataBlock(FusBackendApi_t* api, size_t need_realoc, size_t buffer_size)
{
    if (unlikely(!api || need_realoc == 0 || buffer_size == 0)) return NULL;

    FusBackendGenerateDataBlock_t* block = api->FusAlloc(api,sizeof(FusBackendGenerateDataBlock_t));
    if (unlikely(!block)) return NULL;

    FusBackendRelocationNeed_t* reallocs = api->FusAlloc(api,sizeof(FusBackendRelocationNeed_t) * need_realoc);
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

    block->reloc_capacity = need_realoc;
    block->reloc_count = 0;
    block->reloc = reallocs;

    block->buffer_slab = buffer;
    block->arena = arena;
    block->flag = FUSION_ERRO;

    block->slab_size = buffer_size;
    block->slab_offset = 0;

    block->api = api;
    return block;
}
static void FUSI_DestroyDataBlock(FusBackendApi_t* api, FusBackendGenerateDataBlock_t* block)
{
    if (unlikely(!api || !block)) return;

    FUSI_DestroyArena(block->arena);
    api->FusFree(api,block->buffer_slab);
    api->FusFree(api,block->reloc);
    api->FusFree(api,block);
}
static FusStatusFlag_t FUSI_RegistreRealocationDataBlock(FusBackendApi_t* api, 
    FusBackendGenerateDataBlock_t* block, const char* name, FusBackendRelocationOpaqueType_t type, size_t offset
)
{
    if (unlikely(!api || !block || !name)) return FUSION_ERRO;
    if (block->reloc_count >= block->reloc_capacity) return FUSION_ERRO;

    FusBackendRelocationNeed_t* realoc_new = &block->reloc[block->reloc_count];

    char* arena_name = FUSI_ArenaPushString(block->arena,name);
    if (!arena_name) return FUSION_ERRO;
    realoc_new->name = arena_name;
    realoc_new->type = type;
    realoc_new->offset = offset;

    block->reloc_count++;
    return FUSION_OK;
}


static FusBackendApi_t interface_api = {
    .FusAlloc = FUSI_BackendHookMalloc,
    .FusFree = FUSI_BackendHookFree,
    .FusCreateTransfer = FUSI_CreateTransferLifetime,
    .FusDestroyTransfer = FUSI_DestroyTransferLifetime,
    .FusCreateDataBlock = FUSI_CreateDataBlock,
    .FusDestroyDataBlock = FUSI_DestroyDataBlock,
    .FusRegistreRealocationDataBlock = FUSI_RegistreRealocationDataBlock
};
FusBackendApi_t FUSI_InterfaceDefine()
{
    return interface_api;
}