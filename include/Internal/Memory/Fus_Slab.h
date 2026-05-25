#ifndef FUSION_INTERNAL_SLAB_H
#define FUSION_INTERNAL_SLAB_H
#include <Fusion/FusionTypes.h>

#include <stddef.h>

typedef struct FusSlab FusSlab_t;

FusSlab_t* FUSI_CreateSlab(
    FusInstanceMyAllocation_t* allocation,
    size_t initial_slots,
    size_t min_slots,
    size_t min_size,
    size_t max_size
);
void* FUSI_AllocSlab(FusSlab_t* ctx, size_t size);
void FUSI_FreeSlab(FusSlab_t* ctx, void* ptr);
void FUSI_DestroySlab(FusSlab_t* ctx);

#endif