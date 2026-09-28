/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    backend_dynamic.c
 * @brief   Dynamic backend loading.
 * @author     Ewerton23929dev
 *
 * @details
 * Opens the backend library at runtime, resolves the interface entry point and
 * keeps the handle for later teardown.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <Fusion/FusionTypes.h>
#include <BackendInterface/Backend.h>

// LOCAL
#include "backend_internal.h"

#include <dlfcn.h>
#include <stdio.h>

FusStatusFlag_t fusiLoaderDynamicBackend(const char* path, FusBackendDynamic* out)
{
    if (unlikely(!path || !out)) return FUSION_ERRO;
    void* handle = dlopen(path,RTLD_NOW);
    if (unlikely(!handle)) return FUSION_ERRO;

    FusBackendInterfaceDefine_t ModuleBackendDefine = (FusBackendInterfaceDefine_t)dlsym(handle,"FusCreateBackend");
    char* error = dlerror();
    if (unlikely(error != NULL)) {
        fprintf(stderr, "Erro to found backend init: %s\n",error);
        dlclose(handle);
        return FUSION_ERRO;
    }

    out->handle = handle;
    out->interface = ModuleBackendDefine();
    return FUSION_OK;
}
void fusiDestroyDynamicBackend(FusBackendDynamic* dyn)
{
    if (unlikely(!dyn)) return;

    dlclose(dyn->handle);
    dyn->handle = NULL;
    dyn->interface = NULL;
}