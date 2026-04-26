#ifndef FUSION_BACKEND_MODULE_H
#define FUSION_BACKEND_MODULE_H

typedef enum {
    FUS_BACKEND_TYPE_NONE,
    FUS_BACKEND_TYPE_STATIC,
    FUS_BACKEND_TYPE_DINAMIC, // AINDA NAO EXISTE
} FusModuleBackendType_t;
typedef struct FusModuleBackend FusModuleBackend_t;
typedef struct FusBackendReturn FusBackendReturn_t;

FusModuleBackend_t* FUS_LoaderBackend(const char* name, FusModuleBackendType_t type);
void FUS_DestroyBackend(FusModuleBackend_t* backend);
#endif