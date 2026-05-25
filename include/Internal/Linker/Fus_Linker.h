#ifndef FUSION_INTERNAL_LINKER_H
#define FUSION_INTERNAL_LINKER_H
#include <Fusion/Linker/FusionLinkerInterface.h>

#include <Internal/Fus_Backend.h>

#include "Fus_Hashtable.h"


typedef struct {
    FusLinkerContextSymbol_t* symbol;
    FusBackendRealocOpaqueType_t type;

    size_t section_index;
    size_t offset;
} FusLinkerContextReloc_t;

struct FusLinkerContext {
    FusLinkerContextSection_t* sections;
    size_t sections_count;
    FusLinkerContextSymbol_t* symbols;
    size_t symbols_count;
    FusLinkerContextReloc_t* realocs;
    size_t realocs_count;

    FdbHashTable_t* section_table;
    FdbHashTable_t* symbols_table;
    FusMemoryArena_t* arena;
};

#endif