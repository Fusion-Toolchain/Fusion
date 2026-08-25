#ifndef FUSION_INSTANCE_H
#define FUSION_INSTANCE_H
#include <Fusion/FusionTypes.h>

FusStatusFlag_t fusCreateInstance(FusInstance* ctx,FusInstanceMyAllocation_t* allocation);
FusStatusFlag_t fusDestroyInstance(FusInstance ctx);
#endif