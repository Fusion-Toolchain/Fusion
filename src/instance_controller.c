/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    instance_controller.c
 * @brief   Engine instance lifecycle controller.
 * @author     Ewerton23929dev
 *
 * @details
 * Creates and destroys the engine instance, allocating the handle tables, the
 * error tree and the internal allocators, and returning every resource on
 * destruction.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

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

FusStatusFlag_t fusCreateInstance(FusInstance* ctx, FusInstanceMyAllocation_t* allocation)
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

    FusSlab_t* slab = fusiCreateSlab(alloc, 4096, 4, 8, 4096);
    if (unlikely(!slab)) {
        FUSIH_FREE(alloc,ctx_real);

        return FUSION_ERRO;
    }
    FusLargerBlock_t larger_blocks = NULL;
    if (unlikely(fusiInitLargerBlocks(alloc,&larger_blocks,(3*1024*1024)) != FUSION_OK)) {
        FUSIH_FREE(alloc,ctx_real);
        fusiDestroySlab(slab);

        return FUSION_ERRO;
    }

    if (unlikely(fusiInitHandleSystem(&ctx_real->table) != FUSION_OK)) {
        FUSIH_FREE(alloc,ctx_real);
        fusiDestroySlab(slab);
        fusiCloseLargerBlocks(alloc,&larger_blocks);

        return FUSION_ERRO;
    }

    if (unlikely(fusiCreateTraceContext(alloc,&ctx_real->trace) != FUSION_OK)) {
        FUSIH_FREE(alloc,ctx_real);
        fusiDestroySlab(slab);
        fusiCloseLargerBlocks(alloc,&larger_blocks);
        fusiCloseHandleSystem(&ctx_real->table);
        
        return FUSION_ERRO;
    }

    ctx_real->allocation = alloc;
    ctx_real->slab = slab;
    ctx_real->larger_alloc = larger_blocks;

    FUS_PUSH_ERR(ctx_real->trace,FUSION_OK,"Core Instance Create");
    *ctx = ctx_real;
    return FUSION_OK;
}
FusStatusFlag_t fusDestroyInstance(FusInstance ctx)
{
    if (unlikely(!ctx)) return FUSION_ERRO;
    FusInstanceMyAllocation_t* allocation = FUSIH_INSTANCE_GET_ALLOC(ctx);

    #ifdef FUSION_DEBUG
    FUSI_SlabTrace(ctx_real->slab);
    #endif

    fusiDestroyTraceContext(allocation,&ctx->trace);
    fusiCloseHandleSystem(&ctx->table);
    fusiDestroySlab(ctx->slab);
    fusiCloseLargerBlocks(allocation,&ctx->larger_alloc);
    FUSIH_FREE(allocation,ctx);

    return FUSION_OK;
}

const char* fusStrError(FusStatusFlag_t status)
{
    switch(status) {
        case FUSION_OK: return "OK";
        case FUSION_ERRO: return "Generic error";
        default: return "Unknown FusionStatusFlag";
    }
}