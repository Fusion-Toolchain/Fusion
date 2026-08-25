#ifndef FUSION_BUFFER_H
#define FUSION_BUFFER_H
#include <Fusion/FusionTypes.h>
#include <Fusion/IO/FusionGenericIO.h>

/*
 * @breif Create Buffer
 * @param size_t Buffer Size
 * @return FusBufferContext_t* Buffer Access
*/
FusBufferContext_t* fusCreateBufferCode(size_t buffer_size);
FusStatusFlag_t fusExecutableBuffer(FusBufferContext_t* buffer);
FusStatusFlag_t fusBufferIOSink(FusBufferContext_t* buffer, FusIOSink sink);
void fusReUsedBuffer(FusBufferContext_t* buffer);
/*
 * @brief Destroy Buffer Access
*/
void fusDestroyBufferCode(FusBufferContext_t* buffer);

#endif