#ifndef FUSION_INTERNAL_IO_SYSTEM_H
#define FUSION_INTERNAL_IO_SYSTEM_H
#include <Fusion/FusionTypes.h>
#include <Fusion/IO/FusionFileIO.h>

#include <stddef.h>

struct FusIOBackend {
    FusStatusFlag_t (*write)(void* ctx, const void* data, size_t size);
    FusStatusFlag_t (*flush)(void* ctx);
    FusStatusFlag_t (*seek)(void* ctx, size_t offset);
    void            (*close)(void* ctx);
    void*           ctx;
};

FusIOBackend_t* IO_CreateGenericIOBackend(void* ctx_data);
#endif