/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    buffer_mounter.c
 * @brief   Code buffer creation and reuse.
 * @author     Ewerton23929dev
 *
 * @details
 * Reserves anonymous memory, marks it executable, computes the entry point and
 * allows the same buffer to be reused across compilations without
 * reallocating.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#include <Fusion/FusionTypes.h>
#include <Fusion/FusionBuffer.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Instance.h>

#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

static inline int fus_memfd_create(const char* name)
{
    return (int)syscall(SYS_memfd_create, name, 1); // 1 = MFD_CLOEXEC
}

FusBufferContext_t* fusCreateBufferCode(FusInstance instance, size_t buffer_size)
{
    if (unlikely(buffer_size == 0)) return NULL;
    FusBufferContext_t* ctx = FUSIH_INSTANCE_ALLOC(&instance,sizeof(FusBufferContext_t));
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
        FUSIH_INSTANCE_FREE(&instance,ctx);
        return NULL;
    }

    ctx->buffer = buffer;
    ctx->buffer_size = aligned_size;
    ctx->offset = 0;

    return ctx;
}
void fusReUsedBuffer(FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return;
    buffer->offset = 0;
}

FusStatusFlag_t fusExecutableBuffer(FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return FUSION_ERRO;

    int fd = fus_memfd_create("fusion_jit");
    if (fd < 0) {
        return FUSION_ERRO;
    }

    if (write(fd, buffer->buffer, buffer->offset) != (ssize_t)buffer->offset) {
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
        return FUSION_ERRO;
    }

    buffer->buffer = exec;
    return FUSION_OK;
}
void fusDestroyBufferCode(FusInstance instance, FusBufferContext_t* buffer)
{
    if (unlikely(!buffer)) return;

    munmap(buffer->buffer,buffer->buffer_size);
    FUSIH_INSTANCE_FREE(&instance, buffer);
}