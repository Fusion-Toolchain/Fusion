#include <Fusion/IO/FusionFileIO.h>
#include "io_interface.h"

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
        if (w <= 0) return FUSION_ERRO;
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
    if (lseek(f->fd, (off_t)offset, SEEK_SET) == (off_t)-1) {
        return FUSION_ERRO;
    }
    return FUSION_OK;
}

FusIOBackend_t* FUS_IOBackendFile(const char* path)
{
    if (!path) return NULL;

    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) return NULL;

    IoFile_t* fctx = malloc(sizeof(IoFile_t));
    if (!fctx) {
        close(fd);
        return NULL;
    }

    fctx->fd = fd;

    FusIOBackend_t* backend = IO_CreateGenericIOBackend(fctx);
    if (!backend) {
        close(fd);
        free(fctx);
        return NULL;
    }

    backend->write = _file_write;
    backend->flush = _file_flush;
    backend->close = _file_close;
    backend->seek = _file_seek;

    return backend;
}