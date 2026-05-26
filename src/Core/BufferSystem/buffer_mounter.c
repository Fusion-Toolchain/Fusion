#include <Fusion/FusionTypes.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

FusBufferContext_t* FUS_CreateBufferCode(size_t buffer_size)
{
    if (unlikely(buffer_size == 0)) return NULL;

    FusBufferContext_t* ctx = malloc(sizeof(FusBufferContext_t));
    if (unlikely(!ctx)) return NULL;

    unsigned char* buffer = mmap(
        NULL,
        buffer_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (unlikely(buffer == MAP_FAILED)) {
        free(ctx);
        return NULL;
    }

    ctx->buffer = buffer;
    ctx->buffer_size = buffer_size;
    ctx->offset = 0;

    return ctx;
}
void FUS_ExecutableBuffer(FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return;
    mprotect(buffer->buffer,buffer->buffer_size,PROT_READ | PROT_EXEC);
}
void FUS_DestroyBufferCode(FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return;

    munmap(buffer->buffer,buffer->buffer_size);
    free(buffer);
}