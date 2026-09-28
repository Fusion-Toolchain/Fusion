/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    io_interface.c
 * @brief   Creation of a generic IO sink.
 * @author     Ewerton23929dev
 *
 * @details
 * Allocates and initializes the sink context from a user supplied vtable, and
 * frees the matching memory on destruction.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#include <Internal/IO/Fus_GenericIO.h>

#include <stdlib.h>

FusStatusFlag_t fusiIOCreateGenericIOSink(FusIOSink* out,FusIOSinkInterfaceDefine interface,void* ctx_data)
{
    if (!out) return FUSION_ERRO;
    struct FusIOSink_T* ctx = malloc(sizeof(struct FusIOSink_T));
    if (!ctx) return FUSION_ERRO;

    ctx->interface = interface;
    ctx->ctx = ctx_data;

    *out = ctx;
    return FUSION_OK;
}
void fusDestroyIOSink(FusIOSink ctx)
{
    if (!ctx) return;
    FusIOSinkInterfaceDefine interface = ctx->interface;
    if (interface.close) interface.close(ctx->ctx);
    free(ctx);
}