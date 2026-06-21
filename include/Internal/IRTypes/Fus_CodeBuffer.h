#ifndef FUSION_PRIVATE_CODE_BUFFER_H
#define FUSION_PRIVATE_CODE_BUFFER_H
#include <Fusion/IRTypes/HidrHelper.h>

struct FusCodeMount_T {
    uint32_t code_count;
    uint32_t code_capacity;

    FusHidrNode_t* code_arry;
    struct FusInstance_T* ref_ctx;
};

#endif