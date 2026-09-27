/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    io_file.c
 * @brief   Implementation of the file sink.
 * @author     Ewerton23929dev
 *
 * @details
 * Materializes the generic IO vtable over a file descriptor, covering open,
 * write, seek, flush and close.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#include <Fusion/IO/FusionFileIO.h>
#include <Internal/IO/Fus_GenericIO.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct { // INTERNAL
    int fd;
} IoFile_t;

static FusStatusFlag_t _file_write(void* ctx, const void* data, size_t size)
{
    IoFile_t* f = (IoFile_t*)ctx;

    ssize_t total = 0;
    while (total < (ssize_t)size) {
        ssize_t w = write(f->fd, (const char*)data + total, size - total);
        if (unlikely(w <= 0)) return FUSION_ERRO;
        total += w;
    }
    return FUSION_OK;
}
static FusStatusFlag_t _file_flush(void* ctx)
{
    (void)ctx;
    return FUSION_OK;
}
static void _file_close(void* ctx)
{
    IoFile_t* f = (IoFile_t*)ctx;
    close(f->fd);
    free(f);
}
static FusStatusFlag_t _file_seek(void* ctx, size_t offset)
{
    IoFile_t* f = (IoFile_t*)ctx;
    // SEEK_SET move o ponteiro para a posição absoluta a partir do início
    if (unlikely(lseek(f->fd, (off_t)offset, SEEK_SET) == (off_t)-1)) return FUSION_ERRO;
    return FUSION_OK;
}

static FusIOSinkInterfaceDefine file_interface = {
    .close = _file_close,
    .flush = _file_flush,
    .seek = _file_seek,
    .write = _file_write
};

FusStatusFlag_t fusIOFileSink(FusIOSink* out,const char* path)
{
    if (unlikely(!out || !path)) return FUSION_ERRO;

    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (unlikely(fd < 0)) return FUSION_ERRO;

    IoFile_t* fctx = malloc(sizeof(IoFile_t));
    if (unlikely(!fctx)) {
        close(fd);
        return FUSION_ERRO;
    }
    fctx->fd = fd;

    FusIOSink sink = NULL;
    if (unlikely(!fusiIOCreateGenericIOSink(&sink,file_interface,fctx))) {
        close(fd);
        free(fctx);
        return FUSION_ERRO;
    }

    *out = sink;
    return FUSION_OK;
}