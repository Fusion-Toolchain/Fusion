#ifndef FUSION_PUBLIC_TRACE_TREE_H
#define FUSION_PUBLIC_TRACE_TREE_H
#include <Fusion/FusionTypes.h>
#include <stdint.h>

typedef struct FusTraceTree_T* FusTraceTree;

FusStatusFlag_t FUS_PushError(
    FusTraceTree tree_ctx, FusStatusFlag_t code,
    const char* file, uint32_t line, const char* message
);
void FUS_ClearErrors(FusTraceTree tree_ctx);
void FUS_DumpTrace(FusTraceTree tree_ctx);

FusStatusFlag_t FUS_InstanceGetTrace(FusInstance instance, FusTraceTree* out);

#endif