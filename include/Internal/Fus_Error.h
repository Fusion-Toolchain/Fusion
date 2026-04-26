#ifndef FUSION_INTERNAL_ERRO_H
#define FUSION_INTERNAL_ERRO_H
#include <Fusion/FusionErro.h>
#include <Fusion/IRTypes/HidrType.h>

#include <stddef.h>

typedef struct {
    FusHidrNode_t* hidr;
    size_t hidr_index;
} FusTracedErroDataHidr_t;
struct FusTracedErro {
    FusTracedErroType_t type;
    FusTracedErroLocal_t local;
    const char* msg;
    union {
        FusTracedErroDataHidr_t data_hidr;
    } extra_data;
};

#endif