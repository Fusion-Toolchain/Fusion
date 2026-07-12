#ifndef BACKEND_INTERNAL_DEFINES_H
#define BACKEND_INTERNAL_DEFINES_H
#include <Internal/Backend/Fus_Backend.h>

typedef struct {
    void* handle;
    FusBackendInterface_t* interface;
} FusBackendDynamic_t;

FusBackendApi_t FUSI_InterfaceDefine();

FusStatusFlag_t FUSI_LoaderDynamicBackend(const char* path, FusBackendApi_t* api, FusBackendDynamic_t* out);
void FUSI_DestroyDynamicBackend(FusBackendDynamic_t* dyn);

#endif