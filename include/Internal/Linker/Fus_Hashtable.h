#ifndef FUSION_INTERNAL_FDB_HASHTABLE_H
#define FUSION_INTERNAL_FDB_HASHTABLE_H
#include "Fusion/FusionTypes.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <Internal/Memory/Fus_Arena.h>

typedef struct {
    const char* key;
    size_t idx;
    uint32_t hash;
    bool used;
} FdbHashEntry_t;
typedef struct {
    FdbHashEntry_t* entries;
    size_t entries_count;
    size_t capacity;
    FusMemoryArena_t* arena;
} FdbHashTable_t;

FdbHashTable_t* FDBI_HashTableCreate(size_t capacity);
FusStatusFlag_t FDBI_HashTableInsert(FdbHashTable_t* hash_table, const char* key, size_t idx);
bool FDBI_HashTableGet(FdbHashTable_t* hash_table, const char* key, size_t* idx);
void FDBI_HashTableDestroy(FdbHashTable_t* ht);
#endif