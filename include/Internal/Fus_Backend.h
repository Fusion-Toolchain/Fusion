#ifndef FUSION_INTERNAL_BACKEND_H
#define FUSION_INTERNAL_BACKEND_H
#include "Fusion/IRTypes/MirType.h"
#include <Fusion/FusionTypes.h>

typedef struct {
    FusStatusFlag_t (*FUSI_BackendMountMir)(FusBufferContext_t*,FusMirNode_t*);
} FusBackendInterface_t;

FusBackendInterface_t* FUSI_BackendInit();
#endif