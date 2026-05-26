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

static inline size_t FUSI_AlignForward(size_t ptr, size_t align)
{
    size_t mod = ptr & (align - 1);
    if (mod) ptr += (align - mod);

    return ptr;
}

FusMemoryArena_t* FUSI_CreateArena(size_t size)
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
void* FUSI_AllocArena(FusMemoryArena_t* arena, size_t size)
{
    if (unlikely(!arena)) return NULL;

    size_t aligned = FUSI_AlignForward(arena->offset, DEFAULT_ALIGN_ARENA);

    // CHECKS
    if (unlikely(size > arena->size - aligned)) return NULL;

    void* ptr = ((uint8_t*)arena->data + aligned);
    arena->offset = FUSI_AlignForward(aligned + size, DEFAULT_ALIGN_ARENA);

    return ptr;
}

// HELPER
char* FUSI_ArenaPushString(FusMemoryArena_t* arena, const char* str)
{
    if (unlikely(!arena || !str)) return NULL;

    size_t len = strlen(str) + 1;
    char* mem = FUSI_AllocArena(arena, len);
    if (!mem) {
        return NULL;
    }
    memcpy(mem, str, len);

    return mem;
}
char* FUSI_ArenaPrintf(FusMemoryArena_t* arena, const char* fmt, ...)
{
    if (unlikely(!arena || !fmt)) return NULL;

    size_t aligned = FUSI_AlignForward(arena->offset, DEFAULT_ALIGN_ARENA);
    if (unlikely(aligned >= arena->size)) return NULL;

    size_t available = arena->size - aligned;

    va_list args;
    va_start(args, fmt);

    int written = vsnprintf((char*)arena->data + aligned, available, fmt, args);

    va_end(args);

    if (unlikely(written < 0 || (size_t)written >= available)) return NULL;

    char* start = (char*)arena->data + aligned;
    arena->offset = FUSI_AlignForward(aligned + (size_t)written + 1, DEFAULT_ALIGN_ARENA);

    return start;
}

void FUSI_ResetArena(FusMemoryArena_t* arena)
{
    if (unlikely(!arena)) return;

    #ifdef FUSION_DEBUG
        // TRASH DATA, DESTROY!!!!! BOMMMM!!!!
        memset(arena->data, 0xAA, arena->size);
    #endif

    arena->offset = 0;
}
void FUSI_DestroyArena(FusMemoryArena_t* arena)
{
    if (unlikely(!arena)) return;

    free(arena);
}