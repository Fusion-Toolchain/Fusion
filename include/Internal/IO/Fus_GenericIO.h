#ifndef FUSION_INTERNAL_IO_SYSTEM_H
#define FUSION_INTERNAL_IO_SYSTEM_H
#include <Fusion/FusionTypes.h>
#include <Fusion/IO/FusionGenericIO.h>

#include <stddef.h>

typedef struct {
    FusStatusFlag_t (*write)(void* ctx, const void* data, size_t size);
    FusStatusFlag_t (*flush)(void* ctx);
    FusStatusFlag_t (*seek)(void* ctx, size_t offset);
    void            (*close)(void* ctx);
} FusIOSinkInterfaceDefine;
struct FusIOSink_T {
    FusIOSinkInterfaceDefine interface;
    void*           ctx;
};

FusStatusFlag_t FUSI_IOCreateGenericIOSink(FusIOSink* out,FusIOSinkInterfaceDefine interface,void* ctx_data);
#endif