#include <Internal/Backend/Fus_Backend.h>
#include <Internal/Memory/Fus_Arena.h>

#include "x86_helpers.h"

#include <stdbool.h>
#include <stddef.h>

bool X86RegistreRealloc(FusBackendGenerateDataBlock_t* block, const char* name, FusBackendRelocationOpaqueType_t type, size_t offset)
{
    if (!block) return false;
    if (block->reloc_count >= block->reloc_capacity) return false;

    FusBackendRelocationNeed_t* realoc_new = &block->reloc[block->reloc_count];

    char* arena_name = FUSI_ArenaPushString(block->arena,name);
    if (!arena_name) return false;
    realoc_new->name = arena_name;
    realoc_new->type = type;
    realoc_new->offset = offset;

    block->reloc_count++;
    return true;
}