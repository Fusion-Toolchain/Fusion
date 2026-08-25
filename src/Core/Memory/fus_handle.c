#include <Internal/Memory/Fus_Handle.h>

#include <stdlib.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stddef.h>
#include <stdint.h>

#define FUSION_HANDLE_MAX_GEN 255

typedef struct {
    void* ptr;
    FusDestroyFn_t destroy;
    uint8_t generation;
    uint8_t type;
} FusSlot_t;
struct FusTable_T {
    FusSlot_t* slots;
    uint32_t capacity;
    uint32_t* freelist;
    uint32_t freelist_count;
};

FusStatusFlag_t fusiInitHandleSystem(FusTable_t* table)
{
    if (unlikely(!table)) return FUSION_ERRO;

    size_t total_size = sizeof(struct FusTable_T) +
                        (sizeof(uint32_t) * FUSION_MAX_SLOTS) +
                        (sizeof(FusSlot_t) * FUSION_MAX_SLOTS);

    struct FusTable_T* handle_table = calloc(1,total_size);
    if (unlikely(!handle_table)) return FUSION_ERRO;

    handle_table->freelist = (uint32_t*)(handle_table + 1);
    handle_table->slots = (FusSlot_t*)(handle_table->freelist + FUSION_MAX_SLOTS);

    handle_table->capacity = FUSION_MAX_SLOTS;
    handle_table->freelist_count = FUSION_MAX_SLOTS;

    // MOUNT FREELIST
    for (size_t i = 0; i < FUSION_MAX_SLOTS; i++) {
        handle_table->freelist[i] = FUSION_MAX_SLOTS - 1 - i;
    }
    handle_table->freelist_count = FUSION_MAX_SLOTS;

    *table = handle_table;
    return FUSION_OK;
}
void fusiCloseHandleSystem(FusTable_t* table)
{
    if (unlikely(!table)) return;
    struct FusTable_T* handle_table = *table;

    for (size_t i = 0; i < FUSION_MAX_SLOTS; i++) {
        FusSlot_t* slot = &handle_table->slots[i];
        if (slot->destroy && slot->ptr) slot->destroy(slot->ptr); // ELIMINA TUDO!
    }

    free(handle_table);
    *table = NULL;
}

FusMemoryId_t fusiAllocHandle(FusTable_t* table,void* data, uint8_t type,FusDestroyFn_t destroy)
{
    if (unlikely(!table || !data || !destroy)) return FUSION_INVALID_HANDLE;
    struct FusTable_T* handle_table = *table;

    if (unlikely(handle_table->freelist_count == 0)) return FUSION_INVALID_HANDLE;

    uint32_t id = handle_table->freelist[--handle_table->freelist_count];
    FusSlot_t* slot = &handle_table->slots[id];

    slot->destroy = destroy;
    slot->ptr = data;
    slot->type = type;

    return FUSI_HandleMake(id,type,slot->generation);
}
void* fusiGetDataHandle(FusTable_t* table,FusMemoryId_t handle)
{
    if (unlikely(!table)) return NULL;
    struct FusTable_T* handle_table = *table;

    uint32_t id = FUSI_HandleGetId(handle);
    if (unlikely(id >= handle_table->capacity)) return NULL;

    FusSlot_t* slot = &handle_table->slots[id];
    if (unlikely(slot->generation != FUSI_HandleGetGen(handle))) return NULL;

    return slot->ptr;
}
void fusiFreeHandle(FusTable_t* table,FusMemoryId_t handle)
{
    if (unlikely(!table)) return;

    struct FusTable_T* handle_table = *table;

    uint32_t id = FUSI_HandleGetId(handle);
    if (unlikely(id >= handle_table->capacity)) return;

    FusSlot_t* slot = &handle_table->slots[id];
    if (unlikely(slot->generation != FUSI_HandleGetGen(handle))) return;

    if (slot->destroy && slot->ptr) slot->destroy(slot->ptr);

    slot->ptr = NULL;
    slot->generation++;

    if (unlikely(slot->generation == FUSION_HANDLE_MAX_GEN)) return; 

    handle_table->freelist[handle_table->freelist_count++] = id;
}