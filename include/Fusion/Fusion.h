#ifndef FUSION_INTERFACE_H
#define FUSION_INTERFACE_H
#include "IRTypes/MirType.h"
#include "FusionTypes.h"

FusionBufferContext_t* FUS_CreateBufferCode(size_t buffer_size);
void FUS_DestroyBufferCode(FusionBufferContext_t* ctx_buffer);
FusionStatusFlag_t FUS_MountMirBytes(FusionBufferContext_t* fus_buffer, FusMirNode_t* mir_node);

#endif