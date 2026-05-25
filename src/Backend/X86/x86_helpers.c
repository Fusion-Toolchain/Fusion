#include <Internal/Fus_Backend.h>
#include <Internal/Memory/Fus_Arena.h>

#include "x86_helpers.h"

#include <stdbool.h>
#include <stddef.h>

bool X86RegistreRealloc(FusBackendGenereteDataBlock_t* block, const char* name, FusBackendRealocOpaqueType_t type, size_t offset)
{
    if (!block) return false;
    if (block->realoc_count >= block->realoc_capacity) return false;

    FusBackendReallocNeed_t* realoc_new = &block->realoc[block->realoc_count];

    char* arena_name = FUSI_ArenaPushString(block->arena,name);
    if (!arena_name) return false;
    realoc_new->name = arena_name;
    realoc_new->type = type;
    realoc_new->offset = offset;

    block->realoc_count++;
    return true;
}