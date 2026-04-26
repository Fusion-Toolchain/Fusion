#include "Fusion/FusionTypes.h"
#include "Internal/Linker/Fus_Hashtable.h"
#include "Internal/Memory/Fus_Arena.h"
#include <Fusion/Linker/FusionLinkerInterface.h>
#include <Internal/Linker/Fus_Linker.h>

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#define FUSION_FILE_MAX_SECTIONS 200

/*
 * ------------------ CONFIGURES BASIC INITS --------------------
*/
static inline FusStatusFlag_t ConfigureFileBasic(FusLinkerContext_t* ctx)
{
    if (!ctx) return FUSION_ERRO;

    FusMemoryArena_t* arena = FUSI_CreateArena(1024 * 1024);
    if (!arena) return FUSION_ERRO;

    ctx->arena = arena;

    return FUSION_OK;
}
static inline void DestroyConfigureFileBasic(FusLinkerContext_t* ctx)
{
    if (!ctx) return;

    if (ctx->arena) FUSI_DestroyArena(ctx->arena);
    free(ctx);
}
static inline FusStatusFlag_t ConfigureSection(FusLinkerContext_t* ctx)
{
    if (!ctx) return FUSION_ERRO;

    FusLinkerContextSection_t* sections = malloc(sizeof(FusLinkerContextSection_t)*FUSION_FILE_MAX_SECTIONS);
    if (!sections) return FUSION_ERRO;
    FdbHashTable_t* section_table = FDBI_HashTableCreate(256);
    if (!section_table) {
        free(sections);
        return FUSION_ERRO;
    }
    ctx->sections = sections;
    ctx->section_table = section_table;
    ctx->sections_count = 0;

    return FUSION_OK;
}
static inline void DestroyConfigureSection(FusLinkerContext_t* ctx)
{
    if (!ctx) return;

    if (ctx->sections) free(ctx->sections);
    if (ctx->section_table) FDBI_HashTableDestroy(ctx->section_table);
}

#define FUSION_LINKER_MAX_SYMBOLS 20
static inline FusStatusFlag_t ConfigureSymbols(FusLinkerContext_t* ctx)
{
    if (!ctx) return FUSION_ERRO;

    FusLinkerContextSymbol_t* symbols = malloc(sizeof(FusLinkerContextSymbol_t)*FUSION_LINKER_MAX_SYMBOLS);
    if (!symbols) return FUSION_ERRO;
    FdbHashTable_t* symbols_table = FDBI_HashTableCreate(256);
    if (!symbols_table) {
        free(symbols);
        return FUSION_ERRO;
    }
    ctx->symbols = symbols;
    ctx->symbols_table = symbols_table;
    ctx->symbols_count = 0;

    return FUSION_OK;
}
static inline void DestroyConfigureSymbols(FusLinkerContext_t* ctx)
{
    if (!ctx) return;
    if (ctx->symbols) free(ctx->symbols);
    if (ctx->symbols_table) FDBI_HashTableDestroy(ctx->symbols_table);
}


FusLinkerContext_t* FUS_CreateLinkerContext()
{
    FusLinkerContext_t* ctx = malloc(sizeof(FusLinkerContext_t));
    if (!ctx) return NULL;

    if (ConfigureFileBasic(ctx) != FUSION_OK) {
        free(ctx);
        return NULL;
    }

    if (ConfigureSection(ctx) != FUSION_OK) {
        DestroyConfigureFileBasic(ctx);
        free(ctx);
        return NULL;
    }
    
    if (ConfigureSymbols(ctx) != FUSION_OK) {
        DestroyConfigureFileBasic(ctx);
        DestroyConfigureSection(ctx);
        free(ctx);
        return NULL;
    }

    ctx->realocs = NULL;
    ctx->realocs_count = 0;

    return ctx;
}

FusStatusFlag_t FUS_AddSectionLinker(FusLinkerContext_t* ctx,FusLinkerContextSectionDefine_t* define)
{
    if (!define) return FUSION_ERRO;
    if (!define->name || define->size == 0) return FUSION_ERRO;
    if (ctx->sections_count >= FUSION_FILE_MAX_SECTIONS) return FUSION_ERRO;

    size_t idx = ctx->sections_count;

    FusLinkerContextSection_t* section = &ctx->sections[idx];
    section->size = define->size;
    section->alignment = define->alignment;
    section->name = FUSI_ArenaPushString(ctx->arena,define->name);
    if (!section->name) return FUSION_ERRO;
    section->flag = define->flag;

    FDBI_HashTableInsert(ctx->section_table,section->name,idx);

    ctx->sections_count++;

    return FUSION_OK;
}
FusLinkerContextSection_t* FUS_GetSectionLinker(FusLinkerContext_t* ctx, const char* name)
{
    if (!ctx || !name) return NULL;

    size_t idx;
    if (!FDBI_HashTableGet(ctx->section_table,name,&idx)) return NULL;
    if (idx >= ctx->sections_count) return NULL;

    return &ctx->sections[idx];
}

FusStatusFlag_t FUS_AddSymbolLinker(FusLinkerContext_t* ctx, const char* name,uintptr_t addr)
{
    if (!ctx || !name) return FUSION_ERRO;
    if (addr == 0) return FUSION_ERRO;
    if (ctx->symbols_count >= FUSION_LINKER_MAX_SYMBOLS) return FUSION_ERRO;

    size_t idx = ctx->symbols_count;

    FusLinkerContextSymbol_t* symbol = &ctx->symbols[idx];
    symbol->name = FUSI_ArenaPushString(ctx->arena,name);
    if (!symbol->name) return FUSION_ERRO;
    symbol->local.addr = addr;

    FDBI_HashTableInsert(ctx->symbols_table,name,idx);

    ctx->symbols_count++;

    return FUSION_OK;
}
FusLinkerContextSymbol_t* FUS_GetSymbolLinker(FusLinkerContext_t* ctx, const char* name)
{
    if (!ctx || !name) return NULL;

    size_t idx;
    if (!FDBI_HashTableGet(ctx->symbols_table,name,&idx)) return NULL;
    if (idx >= ctx->symbols_count) return NULL;

    return &ctx->symbols[idx];
}

void FUS_DestroyLinkerContext(FusLinkerContext_t* ctx)
{
    if (!ctx) return;

    DestroyConfigureSymbols(ctx);
    DestroyConfigureSection(ctx);
    DestroyConfigureFileBasic(ctx);
}