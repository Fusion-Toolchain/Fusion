#ifndef FUSION_INTERNAL_MEMORY_H
#define FUSION_INTERNAL_MEMORY_H
#include <stddef.h>
#include <stdint.h>

#include <Fusion/FusionTypes.h>

#define FUSION_INVALID_HANDLE 0xFFFFFFFF
#define FUSION_ID_MASK   0x0000FFFF
#define FUSION_TYPE_MASK 0x00FF0000
#define FUSION_GEN_MASK  0xFF000000
#define FUSION_TYPE_SHIFT 16
#define FUSION_GEN_SHIFT  24

#define FUSION_MAX_SLOTS 65536

/*
 * @note Type Represet Handle
 * [ gen: 8 | tipo:8 | id:16 ]
*/
typedef uint32_t FusMemoryId_t;

typedef void (*FusDestroyFn_t)(void* data);

/*
 * @brief Create handle
 * @param uint16_t Internal Id
 * @param uint8_t Represent Data Type
 * @param uint8_t Geration
 * @return FusMemoryId_t Handle
*/
static inline FusMemoryId_t FUSI_HandleMake(uint16_t id, uint8_t type, uint8_t gen)
{
    return ((uint32_t)id) |
           ((uint32_t)type << FUSION_TYPE_SHIFT) |
           ((uint32_t)gen  << FUSION_GEN_SHIFT);
}

/*
 * @brief Get Id Handle
 * @param FusMemoryId_t Handle
 * @return uint16_t Id
*/
static inline uint16_t FUSI_HandleGetId(FusMemoryId_t h)
{
    return (uint16_t)(h & FUSION_ID_MASK);
}
/*
 * @brief Get Type Handle
 * @param FusMemoryId_t Handle
 * @return uint8_t Type
*/
static inline uint8_t FUSI_HandleGetType(FusMemoryId_t h)
{
    return (uint8_t)((h & FUSION_TYPE_MASK) >> FUSION_TYPE_SHIFT);
}
/*
 * @brief Get Generation Handle
 * @param FusMemoryId_t Handle
 * @return uint8_t Generation
*/
static inline uint8_t FUSI_HandleGetGen(FusMemoryId_t h)
{
    return (uint8_t)((h & FUSION_GEN_MASK) >> FUSION_GEN_SHIFT);
}
/*
 * @brief Init Handle System
 * @return Return Flag Status
*/
FusStatusFlag_t FUSI_InitHandleSystem();
/*
 * @brief Create Handle to Data
 * @param void* data
 * @param uint8_t type
 * @return Return Handle
*/
FusMemoryId_t FUSI_AllocHandle(void* data, uint8_t type,FusDestroyFn_t destroy);
/*
 * @brief Get Handle Data
 * @param FusMemoryId_t handle
 * @return Handle Data
*/
void* FUSI_GetDataHandle(FusMemoryId_t handle);
/*
 * @brief Free Handle
 * @param FusMemoryId_t handle
 * @note Trasform Handle in Invalid!
*/
void FUSI_FreeHandle(FusMemoryId_t handle);
/*
 * @brief Close Handle System
*/
void FUSI_CloseHandleSystem();

#endif