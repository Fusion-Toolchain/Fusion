#include <Fusion/FusionTypes.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

static inline int fus_memfd_create(const char* name)
{
    return (int)syscall(SYS_memfd_create, name, 1); // 1 = MFD_CLOEXEC
}

FusBufferContext_t* FUS_CreateBufferCode(size_t buffer_size)
{
    if (unlikely(buffer_size == 0)) return NULL;

    FusBufferContext_t* ctx = malloc(sizeof(FusBufferContext_t));
    if (unlikely(!ctx)) return NULL;

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) page_size = 4096; // Fallback

    size_t aligned_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    unsigned char* buffer = mmap(
        NULL,
        aligned_size,
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
    ctx->buffer_size = aligned_size;
    ctx->offset = 0;

    return ctx;
}
void FUS_ReUsedBuffer(FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return;
    buffer->offset = 0;
}

FusStatusFlag_t FUS_ExecutableBuffer(FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return FUSION_ERRO;

    // cria FD anônimo
    int fd = fus_memfd_create("fusion_jit");
    if (fd < 0) {
        fprintf(stderr, "memfd_create falhou: %s\n", strerror(errno));
        return FUSION_ERRO;
    }

    // escreve o código gerado no FD
    if (write(fd, buffer->buffer, buffer->offset) != (ssize_t)buffer->offset) {
        fprintf(stderr, "write falhou: %s\n", strerror(errno));
        close(fd);
        return FUSION_ERRO;
    }

    munmap(buffer->buffer, buffer->buffer_size);
    void* exec = mmap(NULL, buffer->buffer_size,
                      PROT_READ | PROT_EXEC,
                      MAP_SHARED, fd, 0
    );
    close(fd);

    if (exec == MAP_FAILED) {
        fprintf(stderr, "mmap falhou: %s\n", strerror(errno));
        return FUSION_ERRO;
    }

    buffer->buffer = exec;
    return FUSION_OK;
}
void FUS_DestroyBufferCode(FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return;

    munmap(buffer->buffer,buffer->buffer_size);
    free(buffer);
}