/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    fus_larger_block.c
 * @brief   Implementation of the large block allocator.
 * @author     Ewerton23929dev
 *
 * @details
 * Reserves the blocks that exceed the slot size, keeps the list of available
 * blocks and reuses the memory after release.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#include "Fusion/FusionTypes.h"
#include <Internal/Fus_Instance.h>
#include <Internal/Memory/Fus_LargerBlocks.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Allocation.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stddef.h>

typedef struct block_t {
    size_t          size;   // tamanho do bloco (sem o header)
    size_t          free;   // 1 = livre, 0 = ocupado

    struct block_t* next;   // próximo bloco na lista
} block_t;

struct FusLargerBlock_T {
    block_t* blocks;
    size_t pool_size;
};

#define LARGE_BLOCK_MIN_SPLIT 64
#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

FusStatusFlag_t fusiInitLargerBlocks(FusInstanceMyAllocation_t* alloc, FusLargerBlock_t* ctx ,size_t pool_size)
{
    if (unlikely(!alloc || !ctx || pool_size <= sizeof(block_t))) return FUSION_ERRO;

    struct FusLargerBlock_T* ctx_real = FUSIH_ALLOC(alloc,sizeof(struct FusLargerBlock_T));
    if (unlikely(!ctx_real)) return FUSION_ERRO;

    void* pool = FUSIH_ALLOC(alloc, pool_size);
    if (unlikely(!pool)) {
        FUSIH_FREE(alloc,ctx_real);
        return FUSION_ERRO;
    }

    block_t* g_head = (block_t*)pool;
    g_head->size = pool_size - sizeof(block_t);
    g_head->free = 1;
    g_head->next = NULL;

    ctx_real->blocks = g_head;
    ctx_real->pool_size = pool_size;

    *ctx = ctx_real;
    return FUSION_OK;
}

void* fusiAllocLargerBlocks(FusLargerBlock_t* ctx,size_t size)
{
    if (unlikely(!ctx || size == 0)) return NULL;
    struct FusLargerBlock_T* ctx_real = *ctx;

    size = ALIGN_UP(size, 8);

    block_t* current = ctx_real->blocks;

    while (current) {
        if (current->free && current->size >= size) {

            // DIVISION CHECK
            if (current->size >= size + sizeof(block_t) + LARGE_BLOCK_MIN_SPLIT) {
                block_t* split = (block_t*)((char*)current + sizeof(block_t) + size);
                split->size = current->size - size - sizeof(block_t);
                split->free = 1;
                split->next = current->next;

                current->size = size;
                current->next = split;
            }

            current->free = 0;

            return (void*)(current + 1);
        }
        current = current->next;
    }

    return NULL;
}
void fusiFreeLargerBlocks(FusLargerBlock_t* ctx, void* ptr)
{
    if (unlikely(!ctx || !ptr)) return;
    struct FusLargerBlock_T* ctx_real = *ctx;

    char* pool_start = (char*)ctx_real->blocks;
    char* pool_end = pool_start + ctx_real->pool_size;
    if (unlikely((char*)ptr < pool_start + sizeof(block_t) || (char*)ptr >= pool_end)) return;

    block_t* block = (block_t*)ptr - 1;
    block->free = 1;

    block_t* current = ctx_real->blocks;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void fusiCloseLargerBlocks(FusInstanceMyAllocation_t* alloc, FusLargerBlock_t* ctx)
{
    if (unlikely(!alloc || !ctx || !*ctx)) return;
    struct FusLargerBlock_T* ctx_real = *ctx;

    FUSIH_FREE(alloc,ctx_real->blocks);
    FUSIH_FREE(alloc,ctx_real);

    *ctx = NULL;
}