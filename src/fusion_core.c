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

const char* FUS_StrError(FusionStatusFlag_t status)
{
    switch(status) {
        case FUSION_OK: return "OK";
        case FUSION_ERRO: return "Generic error";
        case FUSION_INVALID_OPCODE: return "Invalid opcode";
        case FUSION_INVALID_OPERAND: return "Invalid operand";
        case FUSION_BUFFER_OVERFLOW: return "Buffer overflow";
        case FUSION_NOT_IMPLEMENTED: return "Feature not implemented";
        default: return "Unknown FusionStatusFlag";
    }
}