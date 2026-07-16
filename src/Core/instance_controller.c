#include <Internal/Memory/Fus_Handle.h>
#include <Internal/Memory/Fus_Slab.h>
#include <Internal/Fus_TraceTree.h>
#include <Internal/Memory/Fus_LargerBlocks.h>
#include <Internal/Fus_Instance.h>

// HELPERS
#include <Internal/Helpers/Fus_Helper_Instance.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <Fusion/FusionTypes.h>
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
static void* _Default_Realloc(
    void* userdata,
    void* old_ptr,
    size_t new_size
)
{
    FUS_UNUSED(userdata);
    if (!old_ptr) return NULL;

    #ifdef FUSION_DEBUG
    printf("Global Realloc: %p for size: %lu\n",old_ptr,new_size);
    #endif

    return realloc(old_ptr,new_size);
}

FusStatusFlag_t FUS_CreateInstance(FusInstance* ctx, FusInstanceMyAllocation_t* allocation)
{
    if (unlikely(!ctx)) return FUSION_ERRO;
    *ctx = NULL;

    if (unlikely(allocation && (!allocation->Alloc || !allocation->Free))) return FUSION_ERRO;

    static FusInstanceMyAllocation_t default_alloc = {
        .Alloc = _Default_Malloc,
        .Free  = _Default_Free,
        .Realloc = _Default_Realloc,
        .userdata = NULL
    };

    FusInstanceMyAllocation_t* alloc = allocation ? allocation : &default_alloc;

    struct FusInstance_T* ctx_real = FUSIH_ALLOC(alloc,sizeof(struct FusInstance_T));
    if (unlikely(!ctx_real)) return FUSION_ERRO;

    FusSlab_t* slab = FUSI_CreateSlab(alloc, 4096, 4, 8, 4096);
    if (unlikely(!slab)) {
        FUSIH_FREE(alloc,ctx_real);

        return FUSION_ERRO;
    }
    FusLargerBlock_t larger_blocks = NULL;
    if (unlikely(FUSI_InitLargerBlocks(alloc,&larger_blocks,(3*1024*1024)) != FUSION_OK)) {
        FUSIH_FREE(alloc,ctx_real);
        FUSI_DestroySlab(slab);

        return FUSION_ERRO;
    }

    if (unlikely(FUSI_InitHandleSystem(&ctx_real->table) != FUSION_OK)) {
        FUSIH_FREE(alloc,ctx_real);
        FUSI_DestroySlab(slab);
        FUSI_CloseLargerBlocks(alloc,&larger_blocks);

        return FUSION_ERRO;
    }

    if (unlikely(FUSI_CreateTraceContext(alloc,&ctx_real->trace) != FUSION_OK)) {
        FUSIH_FREE(alloc,ctx_real);
        FUSI_DestroySlab(slab);
        FUSI_CloseLargerBlocks(alloc,&larger_blocks);
        FUSI_CloseHandleSystem(&ctx_real->table);
        
        return FUSION_ERRO;
    }

    ctx_real->allocation = alloc;
    ctx_real->slab = slab;
    ctx_real->larger_alloc = larger_blocks;

    FUS_PUSH_ERR(ctx_real->trace,FUSION_OK,"Core Instance Create");
    *ctx = ctx_real;
    return FUSION_OK;
}
FusStatusFlag_t FUS_DestroyInstance(FusInstance ctx)
{
    if (unlikely(!ctx)) return FUSION_ERRO;
    FusInstanceMyAllocation_t* allocation = FUSIH_INSTANCE_GET_ALLOC(ctx);

    #ifdef FUSION_DEBUG
    FUSI_SlabTrace(ctx_real->slab);
    #endif

    FUSI_DestroyTraceContext(allocation,&ctx->trace);
    FUSI_CloseHandleSystem(&ctx->table);
    FUSI_DestroySlab(ctx->slab);
    FUSI_CloseLargerBlocks(allocation,&ctx->larger_alloc);
    FUSIH_FREE(allocation,ctx);

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