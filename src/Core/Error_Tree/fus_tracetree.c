#include <Internal/Fus_TraceTree.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>

// TYPES
#include <Fusion/FusionTypes.h>
#include <stdint.h>

#define FUS_ERROR_MAX_NODES 32
typedef struct FusErrorNode_t {
    FusStatusFlag_t code;
    const char* file;
    uint32_t line;
    struct FusErrorNode_t* parent;
} FusErrorNode_t;
struct FusTraceTree_T {
    FusErrorNode_t nodes[FUS_ERROR_MAX_NODES];
    uint32_t count;
    FusErrorNode_t* current;
};

FusStatusFlag_t FUSI_CreateTraceContext(FusInstanceMyAllocation_t* allocator,FusTraceTree_t* out)
{
    if (unlikely(!allocator || !out)) return FUSION_ERRO;

    struct FusTraceTree_T* tree = FUSIH_ALLOC(allocator,sizeof(struct FusTraceTree_T));
    if (unlikely(!tree)) return FUSION_ERRO;

    tree->count = 0;
    tree->current = NULL;

    *out = tree;
    return FUSION_OK;
}
void FUSI_DestrotTraceContext(FusInstanceMyAllocation_t* allocator, FusTraceTree_t* tree_ctx)
{
    if (unlikely(!allocator || !tree_ctx)) return;
    struct FusTraceTree_T* tree = *tree_ctx;

    FUSIH_FREE(allocator,tree);
    *tree_ctx = NULL;
}

FusStatusFlag_t FUSI_PushError(FusTraceTree_t* tree_ctx, FusStatusFlag_t code, const char* file, uint32_t line)
{
    if (unlikely(!tree_ctx)) return FUSION_ERRO;
    struct FusTraceTree_T* tree = *tree_ctx;

    if (unlikely(tree->count >= FUS_ERROR_MAX_NODES)) return FUSION_ERRO;

    FusErrorNode_t* node = &tree->nodes[tree->count++];
    node->code = code;
    node->file = file;
    node->line = line;
    node->parent = tree->current;

    tree->current = node;

    return FUSION_OK;
}
void FUSI_ClearErrors(FusTraceTree_t* tree_ctx)
{
    if (unlikely(!tree_ctx)) return;
    struct FusTraceTree_T* tree = *tree_ctx;

    tree->count = 0;
    tree->current = NULL;
}