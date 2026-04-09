#ifndef FUSION_INTERNAL_STATIC_BACKEND_H
#define FUSION_INTERNAL_STATIC_BACKEND_H
#include "Fus_Backend.h"
#include <string.h>

typedef struct {
    const char* name;
    FusBackendInterface_t* (*fn)(void);
} ModuleStaticEntry_t;

#define REGISTER_BACKEND(mod_name, mod_fn) \
    static const ModuleStaticEntry_t __entry_##mod_name \
    __attribute__((used, section(".static_modules_backend"))) = { \
        .name = #mod_name, \
        .fn   = mod_fn \
    };

static inline ModuleStaticEntry_t* FUS_GetStaticBackend(const char* name)
{
    extern ModuleStaticEntry_t __start_static_modules_backend[];
    extern ModuleStaticEntry_t __stop_static_modules_backend[];

    for (ModuleStaticEntry_t* e = __start_static_modules_backend; e < __stop_static_modules_backend; e++) {
        if (strcmp(e->name, name) == 0) return e;
    }

    return NULL;
}

#endif