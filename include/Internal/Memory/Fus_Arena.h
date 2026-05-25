#ifndef FUSION_INTERNAL_ARENA_H
#define FUSION_INTERNAL_ARENA_H
#include <stddef.h>

/*
 * @brief Struct Arena Fusion
*/
typedef struct {
    void* data;
    size_t size;
    size_t offset;
} FusMemoryArena_t;

/*
 * @brief Create Arena
 * @param size_t Request Size
 * @return FusMemoryArena_t* Arena Access
*/
FusMemoryArena_t* FUSI_CreateArena(size_t size);
/*
 * @brief Alloc Item in Arena
 * @param FusMemoryArena_t* Arena Access
 * @param size_t Item Size
 * @return void* Data
*/
void* FUSI_AllocArena(FusMemoryArena_t* arena, size_t size);
/*
 * @brief Clear/Reset Arena
 * @param FusMemoryArena_t* Arena
*/
char* FUSI_ArenaPushString(FusMemoryArena_t* arena, const char* str);
char* FUSI_ArenaPrintf(FusMemoryArena_t* arena, const char* fmt, ...);
void FUSI_ResetArena(FusMemoryArena_t* arena);
/*
 * @brief Destroy Arena
 * @param FusMemoryArena_t* Arena Access
*/
void FUSI_DestroyArena(FusMemoryArena_t* arena);

#endif