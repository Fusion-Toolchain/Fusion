#include <Internal/Memory/Fus_Slab.h>
#include <Internal/Memory/Fus_LargerBlocks.h>
#include <Internal/Fus_Instance.h>

// HELPERS
#include <Internal/Helpers/Fus_Helper_Instance.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void* _Default_Malloc(void* data, size_t size)
{
    FUS_UNUSED(data);

    if (size == 0) return NULL;

    void* ptr = malloc(size);
    #ifdef FUSION_DEBUG
    printf("Global System Allocation: %lu -> %p\n", size, ptr);
    #endif

    return ptr;
}
static void _Default_Free(void* data, void* ptr)
{
    FUS_UNUSED(data);

    if (!ptr) return;

    #ifdef FUSION_DEBUG
    printf("Global System Free: %p\n", ptr);
    #endif

    free(ptr);
}

FusStatusFlag_t FUS_CreateInstance(FusInstance* ctx, FusInstanceMyAllocation_t* allocation)
{
    if (!ctx) return FUSION_ERRO;
    *ctx = NULL;

    if (allocation && (!allocation->Alloc || !allocation->Free)) return FUSION_ERRO;

    static FusInstanceMyAllocation_t default_alloc = {
        .Alloc = _Default_Malloc,
        .Free  = _Default_Free,
        .userdata = NULL
    };

    FusInstanceMyAllocation_t* alloc = allocation ? allocation : &default_alloc;

    struct FusInstance_T* ctx_real = FUSIH_ALLOC(alloc,sizeof(struct FusInstance_T));
    if (!ctx_real) return FUSION_ERRO;

    FusSlab_t* slab = FUSI_CreateSlab(alloc, 4096, 4, 8, 4096);
    if (!slab) {
        FUSIH_FREE(alloc,ctx_real);

        return FUSION_ERRO;
    }
    FusLargerBlock_t larger_blocks = NULL;
    if (FUSI_InitLargerBlocks(alloc,&larger_blocks,(3*1024*1024)) != FUSION_OK) {
        FUSIH_FREE(alloc,ctx_real);
        FUSI_DestroySlab(slab);

        return FUSION_ERRO;
    }

    ctx_real->allocation = alloc;
    ctx_real->slab = slab;
    ctx_real->larger_alloc = larger_blocks;

    FUSI_InitHandleSystem(&ctx_real->table);

    *ctx = ctx_real;
    return FUSION_OK;
}
FusStatusFlag_t FUS_DestroyInstance(FusInstance* ctx)
{
    if (!ctx) return FUSION_ERRO;
    struct FusInstance_T* ctx_real = *ctx;
    FusInstanceMyAllocation_t* allocation = FUSIH_INSTANCE_GET_ALLOC(ctx);

    FUSI_CloseHandleSystem(&ctx_real->table);
    FUSI_DestroySlab(ctx_real->slab);
    FUSI_CloseLargerBlocks(allocation,&ctx_real->larger_alloc);
    FUSIH_FREE(allocation,ctx_real);

    *ctx = NULL;
    return FUSION_OK;
}

const char* FUS_StrError(FusStatusFlag_t status)
{
    switch(status) {
        case FUSION_OK: return "OK";
        case FUSION_ERRO: return "Generic error";
        default: return "Unknown FusionStatusFlag";
    }
}