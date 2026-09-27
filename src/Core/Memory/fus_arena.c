/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    fus_arena.c
 * @brief   Implementation of the arena allocator.
 * @author     Ewerton23929dev
 *
 * @details
 * Manages the single block, the current offset and the alignment of
 * allocations, plus capacity and free memory queries.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#include <Internal/Memory/Fus_Arena.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stdint.h>
#include <stddef.h>

#define DEFAULT_ALIGN_ARENA 8

static inline size_t AlignForward(size_t ptr, size_t align)
{
    size_t mod = ptr & (align - 1);
    if (mod) ptr += (align - mod);

    return ptr;
}

FusMemoryArena_t* fusiCreateArena(size_t size)
{
    if (unlikely(size == 0)) return NULL;

    // SINGLE ALLOCATION
    FusMemoryArena_t* arena = malloc(sizeof(FusMemoryArena_t) + size);
    if (unlikely(!arena)) return NULL;

    arena->data = (void*)(arena + 1);
    arena->size = size;
    arena->offset = 0;

    return arena;
}
void* fusiAllocArena(FusMemoryArena_t* arena, size_t size)
{
    if (unlikely(!arena)) return NULL;

    size_t aligned = AlignForward(arena->offset, DEFAULT_ALIGN_ARENA);

    // CHECKS
    if (unlikely(size > arena->size - aligned)) return NULL;

    void* ptr = ((uint8_t*)arena->data + aligned);
    arena->offset = AlignForward(aligned + size, DEFAULT_ALIGN_ARENA);

    return ptr;
}

// HELPER
char* fusiArenaPushString(FusMemoryArena_t* arena, const char* str)
{
    if (unlikely(!arena || !str)) return NULL;

    size_t len = strlen(str) + 1;
    char* mem = fusiAllocArena(arena, len);
    if (!mem) {
        return NULL;
    }
    memcpy(mem, str, len);

    return mem;
}
char* fusiArenaPrintf(FusMemoryArena_t* arena, const char* fmt, ...)
{
    if (unlikely(!arena || !fmt)) return NULL;

    size_t aligned = AlignForward(arena->offset, DEFAULT_ALIGN_ARENA);
    if (unlikely(aligned >= arena->size)) return NULL;

    size_t available = arena->size - aligned;

    va_list args;
    va_start(args, fmt);

    int written = vsnprintf((char*)arena->data + aligned, available, fmt, args);

    va_end(args);

    if (unlikely(written < 0 || (size_t)written >= available)) return NULL;

    char* start = (char*)arena->data + aligned;
    arena->offset = AlignForward(aligned + (size_t)written + 1, DEFAULT_ALIGN_ARENA);

    return start;
}

void fusiResetArena(FusMemoryArena_t* arena)
{
    if (unlikely(!arena)) return;

    #ifdef FUSION_DEBUG
        // TRASH DATA, DESTROY!!!!! BOMMMM!!!!
        memset(arena->data, 0xAA, arena->size);
    #endif

    arena->offset = 0;
}
void fusiDestroyArena(FusMemoryArena_t* arena)
{
    if (unlikely(!arena)) return;

    free(arena);
}