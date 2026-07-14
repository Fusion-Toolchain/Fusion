#include <Internal/Fus_TraceTree.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Instance.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>

// TYPES
#include <Fusion/FusionTypes.h>
#include <stdio.h>
#include <stdint.h>

#define FUS_ERROR_MAX_NODES 32
#define FUS_ERROR_MSG_MAX 256

typedef struct FusErrorNode_t {
    FusStatusFlag_t code;
    const char* file;
    uint32_t line;
    char message[FUS_ERROR_MSG_MAX];

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

FusStatusFlag_t FUSI_PushError(
    FusTraceTree_t tree_ctx, FusStatusFlag_t code,
    const char* file, uint32_t line, const char* message
)
{
    if (unlikely(!tree_ctx)) return FUSION_ERRO;
    struct FusTraceTree_T* tree = tree_ctx;   // sem *, já é o handle direto
    if (unlikely(tree->count >= FUS_ERROR_MAX_NODES)) return FUSION_ERRO;

    FusErrorNode_t* node = &tree->nodes[tree->count++];
    node->code = code;
    node->file = file;
    node->line = line;
    node->parent = tree->current;
    tree->current = node;

    if (message) {
        snprintf(node->message, FUS_ERROR_MSG_MAX, "%s", message);
    } else {
        node->message[0] = '\0';
    }

    return FUSION_OK;
}

void FUS_ClearErrors(FusTraceTree_t tree_ctx)
{
    if (unlikely(!tree_ctx)) return;
    struct FusTraceTree_T* tree = tree_ctx;
    tree->count = 0;
    tree->current = NULL;
}

static void FUSI_DumpTraceNode(FusErrorNode_t* node, int depth)
{
    if (!node) return;

    printf("%*s[%s:%u] code=%d\n",
           depth * 2, "",
           node->file ? node->file : "???",
           node->line,
           (int)node->code);
    
    if (node->message[0] != '\0') {
        printf("%*s  └─ %s\n",
               depth * 2, "",
               node->message);
    }

    if (node->parent) {
        FUSI_DumpTraceNode(node->parent, depth + 1);
    }
}

void FUS_DumpTrace(FusTraceTree_t tree_ctx)
{
    if (unlikely(!tree_ctx)) {
        printf("(sem trace context)\n");
        return;
    }
    struct FusTraceTree_T* tree = tree_ctx;
    if (!tree->current) {
        printf("(nenhum erro registrado)\n");
        return;
    }
    
    printf("=== Fusion Error Trace (Raiz -> Saida Final) ===\n");
    FUSI_DumpTraceNode(tree->current, 0);
    printf("================================================\n");
}

FusStatusFlag_t FUS_InstanceGetTrace(FusInstance instance, FusTraceTree_t* out)
{
    if (unlikely(!instance || !out)) return FUSION_ERRO;
    *out = FUSIH_INSTANCE_GET_TRACE(&instance);
    return FUSION_OK;
}