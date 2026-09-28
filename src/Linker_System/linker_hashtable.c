/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    linker_hashtable.c
 * @brief   Implementation of the linker hash table.
 * @author     Ewerton23929dev
 *
 * @details
 * Hashes names, inserts and looks up sections and symbols, and keeps the load
 * factor within limits with on demand growth.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#include <Internal/Linker/Fus_Linker.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

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

    while ((c = *str++))hash = ((hash << 5) + hash) + c;
    return hash;
}

static bool HashTableResize(FdbHashTable_t* ht)
{
    if (unlikely(!ht)) return false;

    size_t new_cap = ht->capacity * 2;
    FdbHashEntry_t* new_entries = calloc(new_cap, sizeof(FdbHashEntry_t));
    if (unlikely(!new_entries)) return false;
    
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

    return true;
}

FdbHashTable_t* FDBI_HashTableCreate(size_t capacity)
{
    if (unlikely(capacity == 0)) return NULL;

    FdbHashTable_t* ht = malloc(sizeof(FdbHashTable_t));
    if (unlikely(!ht)) return NULL;

    ht->capacity = capacity;
    ht->entries_count = 0;
    ht->arena = fusiCreateArena(1*1024*1024);
    if (unlikely(!ht->arena)) {
        free(ht);
        return NULL;
    }

    ht->entries = calloc(capacity,sizeof(FdbHashEntry_t));
    if (unlikely(!ht->entries)) {
        fusiDestroyArena(ht->arena);
        free(ht);
        return NULL;
    }
    return ht;
}

FusStatusFlag_t FDBI_HashTableInsert(FdbHashTable_t* ht, const char* key, size_t idx)
{
    if (unlikely(!ht || !key)) goto err;
    uint32_t hash = HashSectionTableGenHash(key);

    size_t i = hash % ht->capacity;
    size_t start = i;

    while (ht->entries[i].used) {
        if (ht->entries[i].hash == hash &&
            strcmp(ht->entries[i].key, key) == 0)
        {
            return FUSION_OK;
        }

        i = (i + 1) % ht->capacity;
        if (unlikely(i == start)) goto err;
    }

    if (ht->entries_count >= ht->capacity * HT_LOAD_FACTOR) {
        if (unlikely(!HashTableResize(ht))) goto err;

        i = hash % ht->capacity;
        while (ht->entries[i].used) {
            i = (i + 1) % ht->capacity;
        }
    }

    const char* k = fusiArenaPushString(ht->arena, key);
    if (unlikely(!k)) goto err;

    ht->entries[i].key = k;
    ht->entries[i].idx = idx;
    ht->entries[i].hash = hash;
    ht->entries[i].used = true;

    ht->entries_count++;
    return FUSION_OK;

err:
    return FUSION_ERRO;
}

bool FDBI_HashTableGet(FdbHashTable_t* hash_table, const char* key, size_t* idx)
{
    if (unlikely(!hash_table || !key || !idx)) return false;
    if (unlikely(hash_table->capacity == 0)) return false;

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
    if (unlikely(!ht)) return;

    if (likely(ht->entries)) free(ht->entries);
    fusiDestroyArena(ht->arena);
    free(ht);
}