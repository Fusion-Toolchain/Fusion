#include <Fusion/Linker/FusionLinkerInterface.h>

#include <Internal/Linker/Fus_Linker.h>
#include <Internal/Fus_Instance.h>
#include <Internal/Memory/Fus_Slab.h>

// HELPERS
#include <Internal/Helpers/Fus_Helper_Allocation.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stddef.h>
#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>

#define FUSION_FILE_MAX_SECTIONS 200

/*
 * ------------------ CONFIGURES BASIC INITS --------------------
*/
static inline FusStatusFlag_t ConfigureFileBasic(FusLinkerContext_t* ctx)
{
    if (unlikely(!ctx)) return FUSION_ERRO;

    FusMemoryArena_t* arena = FUSI_CreateArena(1024 * 1024);
    if (unlikely(!arena)) return FUSION_ERRO;

    ctx->arena = arena;

    return FUSION_OK;
}
static inline void DestroyConfigureFileBasic(FusLinkerContext_t* ctx)
{
    if (unlikely(!ctx)) return;

    if (unlikely(ctx->arena)) FUSI_DestroyArena(ctx->arena);
}
static inline FusStatusFlag_t ConfigureSection(FusInstanceMyAllocation_t* allocator,FusLinkerContext_t* ctx)
{
    if (unlikely(!allocator)) return FUSION_ERRO;
    if (unlikely(!ctx)) return FUSION_ERRO;

    FusLinkerContextSection_t* sections = FUSIH_ALLOC(allocator,sizeof(FusLinkerContextSection_t)*FUSION_FILE_MAX_SECTIONS);
    if (unlikely(!sections)) return FUSION_ERRO;

    FdbHashTable_t* section_table = FDBI_HashTableCreate(256);
    if (unlikely(!section_table)) {
        FUSIH_FREE(allocator,sections);
        return FUSION_ERRO;
    }
    ctx->sections = sections;
    ctx->section_table = section_table;
    ctx->sections_count = 0;

    return FUSION_OK;
}
static inline void DestroyConfigureSection(FusInstanceMyAllocation_t* allocator,FusLinkerContext_t* ctx)
{
    if (unlikely(!allocator)) return;
    if (unlikely(!ctx)) return;

    if (likely(ctx->sections)) FUSIH_FREE(allocator,ctx->sections);
    if (likely(ctx->section_table)) FDBI_HashTableDestroy(ctx->section_table);
}

#define FUSION_LINKER_MAX_SYMBOLS 20
static inline FusStatusFlag_t ConfigureSymbols(FusInstanceMyAllocation_t* allocator,FusLinkerContext_t* ctx)
{
    if (unlikely(!allocator)) return FUSION_ERRO;
    if (unlikely(!ctx)) return FUSION_ERRO;


    FusLinkerContextSymbol_t* symbols = FUSIH_ALLOC(allocator,sizeof(FusLinkerContextSymbol_t)*FUSION_LINKER_MAX_SYMBOLS);
    if (unlikely(!symbols)) return FUSION_ERRO;

    FdbHashTable_t* symbols_table = FDBI_HashTableCreate(256);
    if (unlikely(!symbols_table)) {
        FUSIH_FREE(allocator,symbols);
        return FUSION_ERRO;
    }

    ctx->symbols = symbols;
    ctx->symbols_table = symbols_table;
    ctx->symbols_count = 0;

    return FUSION_OK;
}
static inline void DestroyConfigureSymbols(FusInstanceMyAllocation_t* allocator,FusLinkerContext_t* ctx)
{
    if (unlikely(!allocator)) return;
    if (unlikely(!ctx)) return;

    if (likely(ctx->symbols)) FUSIH_FREE(allocator,ctx->symbols);
    if (likely(ctx->symbols_table)) FDBI_HashTableDestroy(ctx->symbols_table);
}


FusLinkerContext_t* FUS_CreateLinkerContext(FusInstance* instance)
{
    if (unlikely(!instance)) return NULL;
    struct FusInstance_T* instance_real = *instance;
    FusInstanceMyAllocation_t* allocator = instance_real->allocation;
    FusSlab_t* slab = instance_real->slab;

    FusLinkerContext_t* ctx = FUSI_AllocSlab(slab,sizeof(FusLinkerContext_t));
    if (unlikely(!ctx)) return NULL;

    if (unlikely(ConfigureFileBasic(ctx) != FUSION_OK)) {
        FUSI_FreeSlab(slab,ctx);
        return NULL;
    }

    if (unlikely(ConfigureSection(allocator,ctx) != FUSION_OK)) {
        DestroyConfigureFileBasic(ctx);
        FUSI_FreeSlab(slab,ctx);
        return NULL;
    }
    
    if (unlikely(ConfigureSymbols(allocator,ctx) != FUSION_OK)) {
        DestroyConfigureFileBasic(ctx);
        DestroyConfigureSection(allocator,ctx);
        FUSI_FreeSlab(slab,ctx);
        return NULL;
    }

    ctx->realocs = NULL;
    ctx->realocs_count = 0;

    return ctx;
}

FusStatusFlag_t FUS_AddSectionLinker(FusLinkerContext_t* ctx,FusLinkerContextSectionDefine_t* define)
{
    if (unlikely(!define)) return FUSION_ERRO;
    if (unlikely(!define->name || define->size == 0)) return FUSION_ERRO;
    if (unlikely(ctx->sections_count >= FUSION_FILE_MAX_SECTIONS)) return FUSION_ERRO;

    size_t idx = ctx->sections_count;

    FusLinkerContextSection_t* section = &ctx->sections[idx];
    section->size = define->size;
    section->alignment = define->alignment;

    section->name = FUSI_ArenaPushString(ctx->arena,define->name);
    if (unlikely(!section->name)) return FUSION_ERRO;

    section->flag = define->flag;
    FDBI_HashTableInsert(ctx->section_table,section->name,idx);

    ctx->sections_count++;

    return FUSION_OK;
}
FusLinkerContextSection_t* FUS_GetSectionLinker(FusLinkerContext_t* ctx, const char* name)
{
    if (unlikely(!ctx || !name)) return NULL;

    size_t idx;
    if (unlikely(!FDBI_HashTableGet(ctx->section_table,name,&idx))) return NULL;
    if (unlikely(idx >= ctx->sections_count)) return NULL;

    return &ctx->sections[idx];
}

FusStatusFlag_t FUS_AddSymbolLinker(FusLinkerContext_t* ctx, const char* name,uintptr_t addr)
{
    if (unlikely(!ctx || !name)) return FUSION_ERRO;
    if (unlikely(addr == 0)) return FUSION_ERRO;
    if (unlikely(ctx->symbols_count >= FUSION_LINKER_MAX_SYMBOLS)) return FUSION_ERRO;

    size_t idx = ctx->symbols_count;

    FusLinkerContextSymbol_t* symbol = &ctx->symbols[idx];
    symbol->name = FUSI_ArenaPushString(ctx->arena,name);
    if (unlikely(!symbol->name)) return FUSION_ERRO;
    symbol->local.addr = addr;

    FDBI_HashTableInsert(ctx->symbols_table,symbol->name,idx);

    ctx->symbols_count++;
    return FUSION_OK;
}
FusLinkerContextSymbol_t* FUS_GetSymbolLinker(FusLinkerContext_t* ctx, const char* name)
{
    if (unlikely(!ctx || !name)) return NULL;

    size_t idx;
    if (unlikely(!FDBI_HashTableGet(ctx->symbols_table,name,&idx))) return NULL;
    if (unlikely(idx >= ctx->symbols_count)) return NULL;

    return &ctx->symbols[idx];
}

void FUS_DestroyLinkerContext(FusInstance instance,FusLinkerContext_t* ctx)
{
    if (unlikely(!instance || !ctx)) return;
    struct FusInstance_T* instance_real = instance;

    DestroyConfigureSymbols(instance_real->allocation,ctx);
    DestroyConfigureSection(instance_real->allocation,ctx);
    DestroyConfigureFileBasic(ctx);

    FUSI_FreeSlab(instance_real->slab,ctx);
}