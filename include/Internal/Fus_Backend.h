#ifndef FUSION_INTERNAL_BACKEND_H
#define FUSION_INTERNAL_BACKEND_H
#include "Fusion/IRTypes/HidrType.h"
#include <Fusion/FusionTypes.h>

typedef struct {
    FusStatusFlag_t (*FUSI_BackendMountMir)(FusBufferContext_t*,FusHidrNode_t*);
} FusBackendInterface_t;
#endif