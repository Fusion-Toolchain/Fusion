// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <Fusion/FusionTypes.h>
#include <BackendInterface/Backend.h>

// LOCAL
#include "backend_internal.h"

#include <dlfcn.h>
#include <stdio.h>

FusStatusFlag_t fusiLoaderDynamicBackend(const char* path, FusBackendApi_t* api, FusBackendDynamic_t* out)
{
    if (unlikely(!path || !api || !out)) return FUSION_ERRO;
    void* handle = dlopen(path,RTLD_NOW);
    if (unlikely(!handle)) return FUSION_ERRO;

    dlerror();
    FusBackendInterfaceDefine_t ModuleBackendDefine = (FusBackendInterfaceDefine_t)
        dlsym(handle,"FUS_CreateBackend");
    
    char* error = dlerror();
    if (unlikely(error != NULL)) {
        fprintf(stderr, "Erro to found backend init: %s\n",error);
        dlclose(handle);
        return FUSION_ERRO;
    }

    out->handle = handle;
    out->interface = ModuleBackendDefine(api);
    return FUSION_OK;
}
void fusiDestroyDynamicBackend(FusBackendDynamic_t* dyn)
{
    if (unlikely(!dyn)) return;

    dlclose(dyn->handle);

    dyn->handle = NULL;
    dyn->interface = NULL;
}