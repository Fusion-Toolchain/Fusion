#ifndef FUSION_PUBLIC_RULE_H
#define FUSION_PUBLIC_RULE_H
#include "Backend/FusionBackend.h"
#include "IRTypes/HidrType.h"
#include "FusionTypes.h"

#include <stddef.h>

typedef enum {
    FUS_COMMAND_SEND_BUFFER,
    FUS_COMMAND_SEND_HIDR,
    FUS_COMMAND_SEND_BACKEND,
    FUS_COMMAND_SEND_LINKER,
} FusCommandRuleType_t;
typedef struct FusCommandRuleBase {
    FusCommandRuleType_t       sType;
    struct FusRuleBase* pNext;
} FusCommandRuleBase_t;

typedef struct {
    FusCommandRuleType_t sType;
    const FusCommandRuleBase_t* pNext;

    FusBufferContext_t* buffer;
} FusCommandBuffer;
typedef struct {
    FusCommandRuleType_t sType;
    const FusCommandRuleBase_t* pNext;

    FusHidrNode_t* hidr_arry;
    size_t hidr_count;
} FusCommandHidr;
typedef struct {
    FusCommandRuleType_t sType;
    const FusCommandRuleBase_t* pNext;

    FusModuleBackend_t backend;
} FusCommandBackend;
#endif