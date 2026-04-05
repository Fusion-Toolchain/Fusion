#include <Internal/Memory/Fus_Arena.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_ALIGN_ARENA 8

static inline size_t FUSI_AlignForward(size_t ptr, size_t align)
{
    size_t mod = ptr & (align - 1);
    if (mod) ptr += (align - mod);
    return ptr;
}

FusMemoryArena_t* FUSI_CreateArena(size_t size)
{
    if (size == 0) return NULL;

    FusMemoryArena_t* arena = malloc(sizeof(FusMemoryArena_t));
    if (!arena) return NULL;
    void* data = malloc(size);
    if (!data) {
        free(arena);
        return NULL;
    }

    arena->data = data;
    arena->size = size;
    arena->offset = 0;

    return arena;
}
void* FUSI_AllocArena(FusMemoryArena_t* arena, size_t size)
{
    if (!arena) return NULL;

    size_t aligned = FUSI_AlignForward(arena->offset, DEFAULT_ALIGN_ARENA);
    if (size > arena->size - aligned) return NULL;
    if (aligned + size > arena->size) return NULL;

    void* ptr = ((uint8_t*)arena->data + aligned);
    arena->offset =  aligned + size;

    return ptr;
}
void FUSI_ResetArena(FusMemoryArena_t* arena)
{
    if (!arena) return;
    arena->offset = 0;
}
void FUSI_DestroyArena(FusMemoryArena_t* arena)
{
    if (!arena) return;
    free(arena->data);
    free(arena);
}