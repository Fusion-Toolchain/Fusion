/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    backend_internal.h
 * @brief   Internal declarations of the backend loader.
 * @author     Ewerton23929dev
 *
 * @details
 * Boundary between the public loader and its implementations: dynamic library
 * context, interface definition and load and unload functions.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#ifndef BACKEND_INTERNAL_DEFINES_H
#define BACKEND_INTERNAL_DEFINES_H
#include <Internal/Backend/Fus_Backend.h>

typedef struct {
    void* handle;
    FusBackendInterface_t* interface;
} FusBackendDynamic_t;

FusBackendApi_t fusiInterfaceDefine(); // BACKEND INJECT

FusStatusFlag_t fusiLoaderDynamicBackend(const char* path, FusBackendApi_t* api, FusBackendDynamic_t* out);
void fusiDestroyDynamicBackend(FusBackendDynamic_t* dyn);

#endif