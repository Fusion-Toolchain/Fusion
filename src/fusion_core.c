#include <Fusion/FusionTypes.h>
#include <Internal/Fus_Backend.h>
#include <Fusion/IRTypes/MirType.h>

#include <stddef.h>
#include <stdlib.h>

FusBufferContext_t* FUS_CreateBufferCode(size_t buffer_size)
{
    FusBufferContext_t* ctx = malloc(sizeof(FusBufferContext_t));
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
void FUS_DestroyBufferCode(FusBufferContext_t* ctx_buffer)
{
    free(ctx_buffer->buffer);
    free(ctx_buffer);
}

FusStatusFlag_t FUS_MountMirBytes(FusBufferContext_t* fus_buffer, FusMirNode_t* mir_node)
{
    if (!fus_buffer || !mir_node) return FUSION_ERRO;

    FusBackendInterface_t* interface = FUSI_BackendInit();
    return interface->FUSI_BackendMountMir(fus_buffer,mir_node);
}

const char* FUS_StrError(FusStatusFlag_t status)
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