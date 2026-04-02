#include <Fusion/FusionTypes.h>

#include <stddef.h>
#include <stdlib.h>

FusionBufferContext_t* FUS_CreateBufferCode(size_t buffer_size)
{
    FusionBufferContext_t* ctx = malloc(sizeof(FusionBufferContext_t));
    if (!ctx) return NULL;
    unsigned char* buffer = malloc(buffer_size);
    if (!buffer) {
        free(ctx);
        return NULL;
    }
    ctx->buffer = buffer;
    ctx->buffer_size = buffer_size;
    ctx->offset = 0;

    return ctx;
}
void FUS_DestroyBufferCode(FusionBufferContext_t* ctx_buffer)
{
    free(ctx_buffer->buffer);
    free(ctx_buffer);
}