#ifndef FUSION_INTERNAL_TRACE_TREE_H
#define FUSION_INTERNAL_TRACE_TREE_H
#include <Fusion/FusionTypes.h>

#include <stdint.h>

typedef struct FusTraceTree_T* FusTraceTree_t;

FusStatusFlag_t FUSI_CreateTraceContext(FusInstanceMyAllocation_t* allocator,FusTraceTree_t* out);
void FUSI_DestrotTraceContext(FusInstanceMyAllocation_t* allocator, FusTraceTree_t* tree_ctx);

FusStatusFlag_t FUSI_PushError(FusTraceTree_t* tree_ctx, FusStatusFlag_t code, const char* file, uint32_t line);
void FUSI_ClearErrors(FusTraceTree_t* tree_ctx);

#define FUS_PUSH_ERR(trace, code) \
    FUSI_PushError(trace, code, __FILE__, __LINE__)

#define FUS_RETURN_ERR(trace, code) \
    do { \
        FUS_PUSH_ERR(trace, code); \
        return FUSION_ERRO; \
    } while(0)
#endif