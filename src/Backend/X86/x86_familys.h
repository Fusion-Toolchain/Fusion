#ifndef X86_INTERNAL_FAMILYS_H
#define X86_INTERNAL_FAMILYS_H
#include <Fusion/IRTypes/HidrType.h>
#include <Internal/Fus_Backend.h>

#include "x86_types.h"

typedef struct {
    x86Instruction_t* encoder;
    const FusHidrNode_t* hidr;
    FusBackendGenereteDataBlock_t* block;
} X86BackendContext;

typedef bool (*X86FmailyRuleFunc_t)(X86BackendContext*);
typedef struct {
    FusHidrOperandType_t src_type;
    FusHidrOperandType_t dst_type;

    X86FmailyRuleFunc_t builder;
} X86FamilyRule_t;
typedef struct {
    X86FamilyRule_t* rules;
    size_t rule_count;

    FusHidrNodeKind_t opcode;
} x86Familys_t;

#endif