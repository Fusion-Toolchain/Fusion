#include <Fusion/Fusion.h>
#include <Fusion/FusionTypes.h>
#include <Fusion/IO/FusionGenericIO.h>
#include <Internal/IO/Fus_GenericIO.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

FusStatusFlag_t fusBufferIOSink(FusBufferContext_t* buffer, FusIOSink sink)
{
    if (unlikely(!buffer || !sink)) return FUSION_ERRO;
    sink->interface.write(sink->ctx,buffer->buffer,buffer->offset);
    return FUSION_OK;
}