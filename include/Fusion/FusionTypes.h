#ifndef FUSION_CORE_TYPES_H
#define FUSION_CORE_TYPES_H
#include <stddef.h>

typedef enum {
    FUSION_OK = 0,
    FUSION_ERRO = 1
} FusStatusFlag_t;
typedef struct {
    unsigned char* buffer;
    size_t buffer_size;
    size_t offset;
} FusBufferContext_t;

#endif