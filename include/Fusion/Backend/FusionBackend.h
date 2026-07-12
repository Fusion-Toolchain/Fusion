#ifndef FUSION_BACKEND_MODULE_H
#define FUSION_BACKEND_MODULE_H
#include <Fusion/FusionTypes.h>

typedef enum {
    FUS_BACKEND_TYPE_NONE,
    FUS_BACKEND_TYPE_STATIC,
    FUS_BACKEND_TYPE_DINAMIC, // AINDA NAO EXISTE
} FusModuleBackendType_t;

FUS_DEFINE_HANDLE(FusModuleBackend)
FUS_DEFINE_HANDLE(FusBackendReturn)

FusStatusFlag_t FUS_LoaderBackend(FusInstance* instance, FusModuleBackend* ctx, const char* name, FusModuleBackendType_t type);
void FUS_DestroyBackend(FusModuleBackend backend);
#endif