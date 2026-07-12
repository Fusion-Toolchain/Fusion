#ifndef FUSION_INTERNAL_INSTANCE_H
#define FUSION_INTERNAL_INSTANCE_H
#include <Fusion/FusionTypes.h>

// SUB-SYSTEM
#include <Internal/Fus_TraceTree.h>
#include <Internal/Memory/Fus_Handle.h>
#include <Internal/Memory/Fus_Slab.h>
#include <Internal/Memory/Fus_LargerBlocks.h>

struct FusInstance_T {
    FusInstanceMyAllocation_t* allocation;

    FusTable_t table;
    FusSlab_t* slab;
    FusLargerBlock_t larger_alloc;
    FusTraceTree_t trace;
};

#endif