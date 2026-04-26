#include <Internal/Linker/Fus_Linker.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define HT_LOAD_FACTOR 0.7

static inline uint32_t HashSectionTableGenHash(const char* str)
{
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

static void HashTableResize(FdbHashTable_t* ht)
{
    size_t new_cap = ht->capacity * 2;
    FdbHashEntry_t* new_entries = calloc(new_cap, sizeof(FdbHashEntry_t));

    for (size_t j = 0; j < ht->capacity; j++) {
        if (!ht->entries[j].used) continue;

        FdbHashEntry_t e = ht->entries[j];
        
        size_t i = e.hash % new_cap;
        while (new_entries[i].used) {
            i = (i + 1) % new_cap;
        }
        new_entries[i] = e;
    }
    free(ht->entries);
    ht->entries = new_entries;
    ht->capacity = new_cap;
}

FdbHashTable_t* FDBI_HashTableCreate(size_t capacity)
{
    if (capacity == 0) return NULL;

    FdbHashTable_t* ht = malloc(sizeof(FdbHashTable_t));
    if (!ht) return NULL;
    ht->capacity = capacity;
    ht->entries_count = 0;
    ht->arena = FUSI_CreateArena(1*1024*1024);
    if (!ht->arena) {
        free(ht);
        return NULL;
    }

    ht->entries = calloc(capacity,sizeof(FdbHashEntry_t));
    if (!ht->entries) {
        FUSI_DestroyArena(ht->arena);
        free(ht);
        return NULL;
    }
    return ht;
}

void FDBI_HashTableInsert(FdbHashTable_t* ht, const char* key, size_t idx)
{
    if (ht->entries_count >= ht->capacity * HT_LOAD_FACTOR) {
        HashTableResize(ht);
    }

    uint32_t hash = HashSectionTableGenHash(key);
    size_t i = hash % ht->capacity;
    size_t start = i;

    while (ht->entries[i].used) {
        if (ht->entries[i].hash == hash &&
            strcmp(ht->entries[i].key, key) == 0)
        {
            ht->entries[i].idx = idx;
            return;
        }

        i = (i + 1) % ht->capacity;
        if (i == start) {
            return;
        }
    }

    const char* k = FUSI_ArenaPushString(ht->arena, key);
    if (!k) return;

    ht->entries[i].key = k;
    ht->entries[i].idx = idx;
    ht->entries[i].hash = hash;
    ht->entries[i].used = true;

    ht->entries_count++;
}
bool FDBI_HashTableGet(FdbHashTable_t* hash_table, const char* key, size_t* idx)
{
    if (!hash_table || !key || !idx) return false;
    if (hash_table->capacity == 0) return false;

    uint32_t hash = HashSectionTableGenHash(key);
    size_t idx_table = hash % hash_table->capacity;
    size_t start = idx_table;

    while (hash_table->entries[idx_table].used) {
        if (hash_table->entries[idx_table].hash == hash && strcmp(hash_table->entries[idx_table].key,key)==0) {
            *idx = hash_table->entries[idx_table].idx;
            return true;
        }
        idx_table = (idx_table + 1) % hash_table->capacity;

        if (idx_table == start) return false;
    }
    return false;
}

void FDBI_HashTableDestroy(FdbHashTable_t* ht)
{
    if (!ht) return;

    if (ht->entries) free(ht->entries);
    FUSI_DestroyArena(ht->arena);
    free(ht);
}