#ifndef FUSION_CORE_TYPES_H
#define FUSION_CORE_TYPES_H
#include <stddef.h>

#define FUS_DEFINE_HANDLE(object) typedef struct object##_T* object;

typedef enum {
    FUSION_OK = 0,
    FUSION_ERRO = 1
} FusStatusFlag_t;
typedef struct {
    void* (*Alloc)(void*,size_t);
    void (*Free)(void*,void*);
    void* (*Realloc)(
        void* userdata,
        void* old_ptr,
        size_t new_size
    );

    void* userdata;
} FusInstanceMyAllocation_t;
typedef struct FusInstance_T* FusInstance;

typedef struct {
    unsigned char* buffer;
    size_t buffer_size;
    size_t offset;
} FusBufferContext_t;

#endif