#ifndef FUSION_INTERFACE_H
#define FUSION_INTERFACE_H

#include "IRTypes/HidrType.h"
#include "Backend/FusionBackend.h"
#include "Linker/FusionLinkerInterface.h"
#include "FusionRule.h"
#include "IRTypes/HidrHelper.h"
#include "IO/FusionGenericIO.h"
#include "FusionBuffer.h"
#include "FusionCompile.h"
#include "FusionInstance.h"

/*
 * @brief Status By String
 * @param FusStatusFlag_t Status Code
 * @return const char* String Error
*/
const char* fusStrError(FusStatusFlag_t status);

#endif