#ifndef FUSION_INTERNAL_HELPER_INSTANCE_H
#define FUSION_INTERNAL_HELPER_INSTANCE_H
#include "Fusion/FusionTypes.h"
#include "Internal/Fus_TraceTree.h"
#include <Internal/Fus_Instance.h>

// ─── Instance Helpers ───────────────────────────────────────────
static inline FusInstanceMyAllocation_t* FUSIH_INSTANCE_GET_ALLOC(FusInstance* instance)
{
    if (!instance) return NULL;
    struct FusInstance_T* real = *instance;
    if (!real) return NULL;

    return real->allocation;
}
static inline void FUSIH_INSTANCE_GET_TRACE(FusInstance* instance, FusTraceTree* trace)
{
    if (!instance) return;
    struct FusInstance_T* real = *instance;
    if (!real) return;
    *trace = real->trace;
}

static inline void* FUSIH_INSTANCE_ALLOC(FusInstance* instance, size_t size)
{
    FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(instance);
    if (!alloc) return NULL;
    return alloc->Alloc(alloc->userdata, size);
}
static inline void FUSIH_INSTANCE_FREE(FusInstance* instance, void* ptr)
{
    FusInstanceMyAllocation_t* alloc = FUSIH_INSTANCE_GET_ALLOC(instance);
    if (!alloc || !ptr) return;
    alloc->Free(alloc->userdata, ptr);
}

#endif