#include "Fusion/FusionTypes.h"
#include <Internal/Memory/Fus_Handle.h>

#include <bits/types/siginfo_t.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

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

FusStatusFlag_t FUSI_InitHandleSystem(FusTable_t* table)
{
    if (!table) return FUSION_ERRO;
    struct FusTable_T* handle_table = calloc(1,sizeof(struct FusTable_T));
    if (!handle_table) {
        return FUSION_ERRO;
    }

    handle_table->capacity = FUSION_MAX_SLOTS;
    handle_table->freelist = malloc(sizeof(uint32_t)*FUSION_MAX_SLOTS);
    if (!handle_table->freelist) {
        free(handle_table);
        return FUSION_ERRO;
    }

    handle_table->slots = malloc(sizeof(FusSlot_t)*FUSION_MAX_SLOTS);
    if (!handle_table->slots) {
        free(handle_table->freelist);
        free(handle_table);
        return FUSION_ERRO;
    }

    for (size_t i = 0; i < FUSION_MAX_SLOTS; i++) {
        handle_table->freelist[i] = FUSION_MAX_SLOTS - 1 - i;

        handle_table->slots[i].ptr = NULL;
        handle_table->slots[i].generation = 0;
        handle_table->slots[i].type = 0;
    }
    handle_table->freelist_count = FUSION_MAX_SLOTS;

    *table = handle_table;
    return FUSION_OK;
}
void FUSI_CloseHandleSystem(FusTable_t* table)
{
    if (!table) return;
    struct FusTable_T* handle_table = *table;

    if (handle_table->freelist) {
        free(handle_table->freelist);
        handle_table->freelist = NULL;
    }
    if (handle_table->slots) {
        free(handle_table->slots);
        handle_table->slots = NULL;
    }

    handle_table->capacity = 0;
    handle_table->freelist_count = 0;

    free(handle_table);
    *table = NULL;
}

FusMemoryId_t FUSI_AllocHandle(FusTable_t* table,void* data, uint8_t type,FusDestroyFn_t destroy)
{
    struct FusTable_T* handle_table = *table;
    if (handle_table->freelist_count == 0) return FUSION_INVALID_HANDLE;

    uint32_t id = handle_table->freelist[--handle_table->freelist_count];
    FusSlot_t* slot = &handle_table->slots[id];

    slot->destroy = destroy;
    slot->ptr = data;
    slot->type = type;

    return FUSI_HandleMake(id,type,slot->generation);
}
void* FUSI_GetDataHandle(FusTable_t* table,FusMemoryId_t handle)
{
    if (!table) return NULL;
    struct FusTable_T* handle_table = *table;

    uint32_t id = FUSI_HandleGetId(handle);
    if (id >= handle_table->capacity) return NULL;

    FusSlot_t* slot = &handle_table->slots[id];
    if (slot->generation != FUSI_HandleGetGen(handle)) return NULL;

    return slot->ptr;
}
void FUSI_FreeHandle(FusTable_t* table,FusMemoryId_t handle)
{
    if (!table) return;
    struct FusTable_T* handle_table = *table;

    uint32_t id = FUSI_HandleGetId(handle);
    if (id >= handle_table->capacity) return;

    FusSlot_t* slot = &handle_table->slots[id];
    if (slot->generation != FUSI_HandleGetGen(handle)) return;

    slot->ptr = NULL;
    slot->generation++;
    if (slot->destroy && slot->ptr) slot->destroy(slot->ptr);

    if (slot->generation == FUSION_HANDLE_MAX_GEN) return; 

    handle_table->freelist[handle_table->freelist_count++] = id;
}