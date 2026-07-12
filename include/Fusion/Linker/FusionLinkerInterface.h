#ifndef FUSION_PUBLIC_LINKER_INTERFACE_H
#define FUSION_PUBLIC_LINKER_INTERFACE_H

#include <Fusion/FusionTypes.h>
#include <Fusion/FusionRule.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t FusLinkerContextSectionFlag_t;
enum {
    FUS_FILE_SECTION_TYPE_READ =  (1 << 0),
    FUS_FILE_SECTION_TYPE_WRITE = (1 << 1),
    FUS_FILE_SECTION_TYPE_EXEC =  (1 << 2),
};

typedef struct {
    const char* name;

    size_t section_index;
    union {
        size_t offset;
        size_t addr;
    } local;
    bool is_global;
} FusLinkerContextSymbol_t;
typedef struct {
    const char* name;
    size_t size;
    size_t offset;
    size_t alignment;
    FusLinkerContextSectionFlag_t flag;
} FusLinkerContextSectionDefine_t;
typedef struct {
    const char* name;
    FusLinkerContextSectionFlag_t flag;

    size_t offset;
    size_t size;
    size_t alignment;
} FusLinkerContextSection_t;

FUS_DEFINE_HANDLE(FusLinkerContext)

FusStatusFlag_t FUS_CreateLinkerContext(FusInstance* instance, FusLinkerContext* out);

FusStatusFlag_t FUS_AddSectionLinker(FusLinkerContext ctx,FusLinkerContextSectionDefine_t* define);
FusLinkerContextSection_t* FUS_GetSectionLinker(FusLinkerContext ctx, const char* name);
FusStatusFlag_t FUS_AddSymbolLinker(FusLinkerContext ctx, const char* name,uintptr_t addr);
FusLinkerContextSymbol_t* FUS_GetSymbolLinker(FusLinkerContext ctx, const char* name);

FusStatusFlag_t FUS_LinkerResolver(FusCommandRuleBase_t* compiler_rule, FusLinkerContext linker_ctx, FusBackendReturn backend_data);
void FUS_DestroyLinkerContext(FusInstance instance, FusLinkerContext ctx);
#endif