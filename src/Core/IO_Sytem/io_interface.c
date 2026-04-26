#include "io_interface.h"

#include <stdlib.h>

FusIOBackend_t* IO_CreateGenericIOBackend(void* ctx_data)
{
    FusIOBackend_t* ctx = malloc(sizeof(FusIOBackend_t));
    if (!ctx) return NULL;

    ctx->close = NULL;
    ctx->flush = NULL;
    ctx->write = NULL;

    ctx->ctx = ctx_data;

    return ctx;
}
void FUS_DestroyIOBackend(FusIOBackend_t* ctx)
{
    if (!ctx) return;
    if (ctx->close) ctx->close(ctx->ctx);
    free(ctx);
}