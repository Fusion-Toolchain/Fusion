#ifndef FUSION_INTERFACE_H
#define FUSION_INTERFACE_H

#include "IRTypes/HidrType.h"
#include "Backend/FusionBackend.h"
#include "Linker/FusionLinkerInterface.h"
#include "FusionRule.h"
#include "FusionTypes.h"

/*
 * @breif Create Buffer
 * @param size_t Buffer Size
 * @return FusBufferContext_t* Buffer Access
*/
FusBufferContext_t* FUS_CreateBufferCode(size_t buffer_size);
void FUS_ExecutableBuffer(FusBufferContext_t* buffer);
/*
 * @brief Destroy Buffer Access
*/
void FUS_DestroyBufferCode(FusBufferContext_t* buffer);
/*
 * @brief Mount Bytes Of Mir
 * @param FusBufferContext_t* Buffer Acess
 * @param FusMirNode_t* Mir Node
 * @return FusStatusFlag_t Build Flag
*/
FusBackendReturn_t* FUS_MountHidrsBytes(FusCommandRuleBase_t* compiler_rule);
/*
 * @brief Status By String
 * @param FusStatusFlag_t Status Code
 * @return const char* String Error
*/
const char* FUS_StrError(FusStatusFlag_t status);

#endif