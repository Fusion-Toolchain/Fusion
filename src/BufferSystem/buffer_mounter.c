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
#include <Internal/Fus_Buffer.h>

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

static inline void* internal_mmap(void* userdata, size_t size)
{
    (void)userdata;

    if (unlikely(size == 0)) return MAP_FAILED;
    return mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,
        0
    );
}
static inline void internal_munmap(void* userdata, void* data, size_t size)
{
    (void)userdata;

    if (unlikely(size == 0)) return;
    munmap(data,size);
}
static FusExecMemAllocator allocator_internal = {
    .userdata = NULL,
    .alloc = internal_mmap,
    .free = internal_munmap
};

static inline FusExecMemAllocator* select_allocator(FusExecMemAllocator* allocator)
{
    if (allocator == NULL) return &allocator_internal;
    return allocator;
}
FusStatusFlag_t fusCreateBufferExecutable(FusInstance instance, FusBufferExecutable* out, size_t buffer_size, FusExecMemAllocator* allocator)
{
    if (unlikely(!out || buffer_size == 0)) return FUSION_ERRO;
    FusExecMemAllocator* current_allocator = select_allocator(allocator);

    struct FusBufferExecutable_T* ctx = FUSIH_INSTANCE_ALLOC(&instance,sizeof(struct FusBufferExecutable_T));
    if (unlikely(!ctx)) return FUSION_ERRO;

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) page_size = 4096; // Fallback
    size_t aligned_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    unsigned char* buffer = current_allocator->alloc(current_allocator->userdata,aligned_size);
    if (unlikely(buffer == MAP_FAILED)) {
        FUSIH_INSTANCE_FREE(&instance,ctx);
        return FUSION_ERRO;
    }

    ctx->data = buffer;
    ctx->size = aligned_size;
    ctx->offset = 0;
    ctx->allocator = current_allocator;
    *out = ctx;

    return FUSION_OK;
}
void fusReUsedBuffer(FusBufferExecutable buffer)
{
    if (unlikely(!buffer)) return;
    buffer->offset = 0;
}

FusStatusFlag_t fusMakeExecutable(FusBufferExecutable buffer)
{
    if (unlikely(!buffer || !buffer->data)) return FUSION_ERRO;
    /*
     * Troca as permissões de RW para RX.
     * Nunca temos W+X ao mesmo tempo.
     */
    if (mprotect(buffer->data, buffer->size, PROT_READ | PROT_EXEC) != 0) return FUSION_ERRO;
    return FUSION_OK;
}
FusStatusFlag_t fusGetExecutableController(FusBufferExecutable buffer, FusBufferController* controller)
{
    if (unlikely(!buffer || !controller)) return FUSION_ERRO;

    controller->data = buffer->data;
    controller->offset = buffer->offset;
    controller->size = buffer->size;

    return FUSION_OK;
}
void fusDestroyBufferExecutable(FusInstance instance, FusBufferExecutable buffer)
{
    if (unlikely(!buffer)) return;

    buffer->allocator->free(buffer->allocator->userdata,buffer->data,buffer->size);
    FUSIH_INSTANCE_FREE(&instance, buffer);
}