#ifndef FUSION_CORE_TYPES_H
#define FUSION_CORE_TYPES_H
#include <stddef.h>

typedef enum {
    FUSION_OK = 0,
    FUSION_ERRO = 1,
    FUSION_INVALID_OPCODE,
    FUSION_INVALID_OPERAND,
    FUSION_BUFFER_OVERFLOW,
    FUSION_NOT_IMPLEMENTED
} FusionStatusFlag_t;
typedef struct {
    unsigned char* buffer;
    size_t buffer_size;
    size_t offset;
} FusionBufferContext_t;

#endif