#ifndef FUSION_INTERNAL_TRACE_TREE_H
#define FUSION_INTERNAL_TRACE_TREE_H
#include <Fusion/FusionTypes.h>

#include <Fusion/FusionTrace.h> // PUBLIC

#include <stdint.h>

FusStatusFlag_t FUSI_CreateTraceContext(FusInstanceMyAllocation_t* allocator,FusTraceTree_t* out);
void FUSI_DestrotTraceContext(FusInstanceMyAllocation_t* allocator, FusTraceTree_t* tree_ctx);

#define FUS_PUSH_ERR(trace, code) \
    FUSI_PushError(trace, code, __FILE__, __LINE__)

#define FUS_RETURN_ERR_VAL(trace, code, retval) \
    do { \
        FUS_PUSH_ERR(trace, code); \
        return (retval); \
    } while(0)

#define FUS_TRACE_AND_GOTO(trace, code, label) \
    do { \
        FUS_PUSH_ERR(trace, code); \
        goto label; \
    } while(0)

#define FUS_RETURN_ERR(trace, code) FUS_RETURN_ERR_VAL(trace, code, FUSION_ERRO)

#endif