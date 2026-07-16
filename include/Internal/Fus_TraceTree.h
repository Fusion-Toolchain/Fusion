#ifndef FUSION_INTERNAL_TRACE_TREE_H
#define FUSION_INTERNAL_TRACE_TREE_H
#include <Fusion/FusionTypes.h>

#include <Fusion/FusionTrace.h> // PUBLIC

#include <stdint.h>

FusStatusFlag_t FUSI_CreateTraceContext(FusInstanceMyAllocation_t* allocator,FusTraceTree* out);
void FUSI_DestrotTraceContext(FusInstanceMyAllocation_t* allocator, FusTraceTree* tree_ctx);

#define FUS_PUSH_ERR(trace, code, msg) \
    FUS_PushError(trace, code, __FILE__, __LINE__, msg)

#define FUS_RETURN_ERR_VAL(trace, code, retval, msg) \
do { \
    FUS_PUSH_ERR(trace, code, msg); \
    return (retval); \
} while(0)

#define FUS_TRACE_AND_GOTO(trace, code, label, msg) \
do { \
    FUS_PUSH_ERR(trace, code, msg); \
    goto label; \
} while(0)

#define FUS_RETURN_ERR(trace, code, msg) \
    FUS_RETURN_ERR_VAL(trace, code, FUSION_ERRO, msg)

#endif