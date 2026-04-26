#include <Internal/Fus_Error.h>

#include <stdlib.h>

FusTracedErro_t* FUS_CreateTracedErro()
{
    FusTracedErro_t* ctx = calloc(0,sizeof(FusTracedErro_t));
    if (!ctx) return NULL;

    ctx->msg = NULL;
    ctx->type = FUS_TRACED_TYPE_NONE;
    ctx->local = FUS_TRACED_LOCAL_NONE;

    return ctx;
}

FusTracedErroType_t FUS_GetTracedErroType(FusTracedErro_t* ctx)
{
    if (!ctx) return FUS_TRACED_TYPE_NONE;
    return ctx->type;
}
FusTracedErroLocal_t FUS_GetTracedErroLocal(FusTracedErro_t* ctx)
{
    if (!ctx) return FUS_TRACED_LOCAL_NONE;
    return ctx->local;
}
const char* FUS_GetTracedErroMsg(FusTracedErro_t* ctx)
{
    if (!ctx) return NULL;
    if (!ctx->msg) return NULL;

    return ctx->msg;
}

void FUS_DestroyTracedErro(FusTracedErro_t* ctx)
{
    free(ctx);
}