#ifndef FUSION_INTERNAL_LARGER_BLOCKS_H
#define FUSION_INTERNAL_LARGER_BLOCKS_H
#include <Fusion/FusionTypes.h>

#include <stddef.h>

typedef struct FusLargerBlock_T* FusLargerBlock_t;

FusStatusFlag_t FUSI_InitLargerBlocks(FusInstanceMyAllocation_t* alloc, FusLargerBlock_t* ctx ,size_t pool_size);
void FUSI_CloseLargerBlocks(FusInstanceMyAllocation_t* alloc, FusLargerBlock_t* ctx);

void* FUSI_AllocLargerBlocks(FusLargerBlock_t* ctx,size_t size);
void FUSI_FreeLargerBlocks(FusLargerBlock_t* ctx, void* ptr);
#endif