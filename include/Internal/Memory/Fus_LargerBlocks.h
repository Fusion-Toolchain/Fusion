#ifndef FUSION_INTERNAL_LARGER_BLOCKS_H
#define FUSION_INTERNAL_LARGER_BLOCKS_H
#include <Fusion/FusionTypes.h>

#include <stddef.h>

/**
 * @brief Contexto Type
 *  Opaque Larger Block Context
 */
typedef struct FusLargerBlock_T* FusLargerBlock_t;

/**
 * @brief Init Larger Bloc Context
 *
 * @warning Use start!
 * 
 * @param alloc 
 * @param ctx 
 * @param pool_size 
 * @return FusStatusFlag_t 
 */
FusStatusFlag_t FUSI_InitLargerBlocks(FusInstanceMyAllocation_t* alloc, FusLargerBlock_t* ctx ,size_t pool_size);
/**
 * @brief Close Larger Block Context
 * 
 * @warning Use for Destroy!
 *
 * @param alloc 
 * @param ctx 
 */
void FUSI_CloseLargerBlocks(FusInstanceMyAllocation_t* alloc, FusLargerBlock_t* ctx);

/**
 * @brief Alloc Block in Larger Block System
 * 
 * @param ctx 
 * @param size 
 * @return void* 
 */
void* FUSI_AllocLargerBlocks(FusLargerBlock_t* ctx,size_t size);
/**
 * @brief Free Larger Block
 * 
 * @param ctx 
 * @param ptr 
 */
void FUSI_FreeLargerBlocks(FusLargerBlock_t* ctx, void* ptr);

#endif // FUSION_INTERNAL_LARGER_BLOCKS_H