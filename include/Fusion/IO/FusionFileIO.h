#ifndef FUSION_IO_FILE_H
#define FUSION_IO_FILE_H
#include <Fusion/FusionTypes.h>
#include "FusionGenericIO.h"

FUS_API FusStatusFlag_t fusIOFileSink(FusIOSink* out,const char* path);
#endif