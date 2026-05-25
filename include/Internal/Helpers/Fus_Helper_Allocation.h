#ifndef FUSION_INTERNAL_HELPER_ALLOCATION_H
#define FUSION_INTERNAL_HELPER_ALLOCATION_H
#include <Internal/Fus_Instance.h>

static inline void* FUSIH_ALLOC(FusInstanceMyAllocation_t* alloc, size_t size)
{
    if (!alloc || size == 0) return NULL;
    return alloc->Alloc(alloc->userdata, size);
}
static inline void FUSIH_FREE(FusInstanceMyAllocation_t* alloc, void* ptr)
{
    if (!alloc || !ptr) return;
    alloc->Free(alloc->userdata,ptr);
}

#endif