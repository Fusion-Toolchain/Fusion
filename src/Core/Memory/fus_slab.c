#include <Internal/Memory/Fus_Slab.h>

// HELPERS
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>

// TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define POOL_NULL_INDEX 0xFFFFFFFF

#define FREELIST_MAGIC 0xEFD9
typedef struct FreeListType_t {
    #ifdef FUSION_DEBUG
    uint16_t magic;
    #endif

    uint32_t next;
} FreeListType_t;

typedef struct {
    FusInstanceMyAllocation_t* allocation;
    void*    memory;       // bloco bruto de memória
    uint32_t free_head;    // inicio da free list

    uint32_t slot_size;    // tamanho de cada slot em bytes
    uint32_t slot_count;   // quantos slots existem
    uint32_t used_count;   // quantos slots estão ocupados agora
    uint32_t used_slots;   // slots usados em toda a vida
} FusPool_t;

struct FusSlab {
    FusInstanceMyAllocation_t* allocation;
    FusPool_t** pools;
    uint32_t pools_num;
};

FusPool_t* FUSI_CreatePool(FusInstanceMyAllocation_t* allocation,size_t num_slots, size_t slot_size)
{
    if (unlikely(!allocation || num_slots == 0 || slot_size == 0)) return NULL;
    if (unlikely(slot_size < sizeof(FreeListType_t))) return NULL;

    size_t size_slots = slot_size * num_slots;
    FusPool_t* ctx = FUSIH_ALLOC(allocation,sizeof(FusPool_t));
    if (unlikely(!ctx)) return NULL;

    void* memory = FUSIH_ALLOC(allocation,size_slots);
    if (unlikely(!memory)) {
        FUSIH_FREE(allocation,ctx);
        return NULL;
    }

    for (size_t i = 0; i < num_slots - 1; i++) {
        FreeListType_t* current = (FreeListType_t*)((uint8_t*)memory + i * slot_size);
        current->next  = (uint32_t)(i + 1);

        #ifdef FUSION_DEBUG
            // FEATURE: DEBUG MODE!
            current->magic = FREELIST_MAGIC;
        #endif
    }
    FreeListType_t* last = (FreeListType_t*)((uint8_t*)memory + (num_slots - 1) * slot_size);
    last->next  = POOL_NULL_INDEX;

    #ifdef FUSION_DEBUG
        // FEATURE: DEBUG MODE!
        last->magic = FREELIST_MAGIC;
    #endif

    ctx->memory = memory;
    ctx->free_head = 0;
    ctx->slot_size = (uint32_t)slot_size;
    ctx->slot_count = (uint32_t)num_slots;
    ctx->used_count = 0;
    ctx->used_slots = 0;
    ctx->allocation = allocation;

    return ctx;
}

void* FUSI_AllocPool(FusPool_t* pool)
{
    if (unlikely(!pool || pool->free_head == POOL_NULL_INDEX)) return NULL;

    FreeListType_t* free_slot = (FreeListType_t*)((uint8_t*)pool->memory + pool->free_head * pool->slot_size);

    #ifdef FUSION_DEBUG
        // FEATURE: DEBUG MODE!
        if (free_slot->magic != FREELIST_MAGIC) return NULL;
        free_slot->magic = 0;
    #endif

    pool->free_head = free_slot->next;
    pool->used_count++;
    pool->used_slots++;

    return (void*)free_slot;
}
void FUSI_FreePool(FusPool_t* pool, void* ptr)
{
    if (unlikely(!pool || !ptr)) return;
    FreeListType_t* free_slot = (FreeListType_t*)ptr;

    #ifdef FUSION_DEBUG
        // FEATURE: DEBUG MODE!
        if (free_slot->magic == FREELIST_MAGIC) return;
    #endif

    size_t offset = (size_t)((uint8_t*)ptr - (uint8_t*)pool->memory);
    uint32_t index = (uint32_t)(offset / pool->slot_size);

    #ifdef FUSION_DEBUG
        // FEATURE: DEBUG MODE!
        free_slot->magic = FREELIST_MAGIC;
    #endif

    free_slot->next = pool->free_head;
    pool->free_head = index;

    pool->used_count--;
}

void FUSI_DestroyPool(FusPool_t* pool)
{
    if (unlikely(!pool)) return;
    FusInstanceMyAllocation_t* allocation = pool->allocation;

    FUSIH_FREE(allocation,pool->memory);
    FUSIH_FREE(allocation,pool);
}


static inline size_t CalcSlabPoolNum(size_t min_size, size_t max_size)
{
    size_t pool_num = 0;
    size_t size = min_size;

    while (size <= max_size) {
        pool_num++;
        size *= 2;
    }

    return pool_num;
}
FusSlab_t* FUSI_CreateSlab(
    FusInstanceMyAllocation_t* allocation,
    size_t initial_slots,
    size_t min_slots,
    size_t min_size,
    size_t max_size
)
{
    if (unlikely(initial_slots == 0 || min_slots == 0 || min_size == 0 || max_size == 0)) return NULL;
    if (unlikely(min_size >= max_size || min_size < sizeof(FreeListType_t))) return NULL;

    FusSlab_t* ctx = FUSIH_ALLOC(allocation,sizeof(FusSlab_t));
    if (unlikely(!ctx)) return NULL;

    size_t pool_num = CalcSlabPoolNum(min_size,max_size);
    FusPool_t** pool_list = FUSIH_ALLOC(allocation,sizeof(FusPool_t*)*pool_num);
    if (unlikely(!pool_list)) {
        FUSIH_FREE(allocation,ctx);
        return NULL;
    }

    size_t slot_size = min_size;
    size_t num_slots = initial_slots;
    for (size_t i = 0; i < pool_num; i++) {
        FusPool_t* new_pool = FUSI_CreatePool(allocation,num_slots,slot_size);

        if (unlikely(!new_pool)) {
            for (size_t j = 0; j < i; j++) FUSI_DestroyPool(pool_list[j]);

            FUSIH_FREE(allocation,pool_list);
            FUSIH_FREE(allocation,ctx);

            return NULL;
        }
        pool_list[i] = new_pool; // SET HERE!!!

        slot_size *= 2;
        num_slots /= 2;

        if (num_slots < min_slots) num_slots = min_slots;
    }

    ctx->pools = pool_list;
    ctx->pools_num = pool_num;
    ctx->allocation = allocation;

    return ctx;
}

void* FUSI_AllocSlab(FusSlab_t* ctx, size_t size)
{
    if (unlikely(!ctx || size == 0)) return NULL;

    for (size_t i = 0; i < ctx->pools_num; i++) {
        FusPool_t* select_pool = ctx->pools[i];
        if (size > select_pool->slot_size) continue;

        if (likely(select_pool->used_count < select_pool->slot_count)) return FUSI_AllocPool(select_pool);
    }

    return NULL;
}
void FUSI_FreeSlab(FusSlab_t* ctx, void* ptr)
{
    if (unlikely(!ctx || !ptr)) return;

    for (size_t i = 0; i < ctx->pools_num; i++) {
        FusPool_t* select_pool = ctx->pools[i];

        size_t pool_bytes = (size_t)select_pool->slot_count * select_pool->slot_size;
        if ((uint8_t*)ptr >= (uint8_t*)select_pool->memory &&
            (uint8_t*)ptr < (uint8_t*)select_pool->memory + pool_bytes
        ) {
            FUSI_FreePool(select_pool,ptr);
            return;
        }
    }
}

void FUSI_SlabTrace(FusSlab_t* ctx)
{
    if (unlikely(!ctx)) return;

    printf("\n\nTrace Slab Called!\n");
    for (size_t i = 0; i < ctx->pools_num; i++) {
        printf(
            "Pool[%ld]: Pool Slot Size: %d, Slots Total: %d, Slots Used End: %d, Slots Used All Life: %d\n"
            ,i,ctx->pools[i]->slot_size,ctx->pools[i]->slot_count,ctx->pools[i]->used_count,ctx->pools[i]->used_slots
        );
    }
}

void FUSI_DestroySlab(FusSlab_t* ctx)
{
    if (unlikely(!ctx)) return;

    for (size_t i = 0; i < ctx->pools_num; i++) {
        FUSI_DestroyPool(ctx->pools[i]);
    }

    FUSIH_FREE(ctx->allocation,ctx->pools);
    FUSIH_FREE(ctx->allocation,ctx);
}