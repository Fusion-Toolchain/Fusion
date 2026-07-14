#include <Internal/Memory/Fus_Slab.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

// TYPES
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#define FUS_TEST_ASSERT(expr, msg) \
    do { \
        if (!(expr)) { \
            printf("❌ FALHA NO TESTE: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while (0)

static void* ExempleAlloc(void* data, size_t size)
{
    (void)data;

    if (size == 0) return NULL;
    return malloc(size);
}
static void ExempleFree(void* data, void* ptr)
{
    (void)data;

    if (!ptr) return;
    free(ptr);
}
static void* ExempleRealloc(void* data, void* old_ptr, size_t size)
{
    (void)data;

    if (!old_ptr || size == 0) return NULL;
    return realloc(old_ptr,size);
}

static FusInstanceMyAllocation_t allocation = {
    .Alloc = ExempleAlloc,
    .Free = ExempleFree,
    .Realloc = ExempleRealloc,
    .userdata = NULL
};

static inline bool Test_SlabMemory()
{
    FusSlab_t* slab = FUSI_CreateSlab(
        &allocation,
        4096,
        8,
        8,
        4096
    );
    FUS_TEST_ASSERT(slab != NULL,"Erro Slab deu erro ao init!");

    int* data = FUSI_AllocSlab(slab,sizeof(int));
    FUS_TEST_ASSERT(data != NULL,"Erro Slab allocou null!");

    *data = 90;
    FUSI_FreeSlab(slab,data);

    FUSI_SlabTrace(slab);
    FUSI_DestroySlab(slab);

    return true;
}

// =============
// TEST MAIN
// =============
int main()
{
    Test_SlabMemory();
}