#include <Internal/Memory/Fus_Slab.h>
#include <Internal/Helpers/Fus_Helper_Codebase.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


#define FUS_TEST_ASSERT(expr, msg) \
    do { \
        if (!(expr)) { \
            printf("❌ FALHA: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
            return false; \
        } \
    } while (0)


// =====================
// TEST ALLOCATOR
// =====================

static size_t alloc_count = 0;
static size_t free_count = 0;


static void* TestAlloc(void* data, size_t size)
{
    (void)data;

    if (size == 0)
        return NULL;

    void* ptr = malloc(size);

    if (ptr)
        alloc_count++;

    return ptr;
}


static void TestFree(void* data, void* ptr)
{
    (void)data;

    if (!ptr)
        return;

    free_count++;
    free(ptr);
}


static void* TestRealloc(void* data, void* old_ptr, size_t size)
{
    (void)data;

    if (size == 0)
        return NULL;

    void* ptr = realloc(old_ptr, size);

    return ptr;
}


static FusInstanceMyAllocation_t TestAllocation =
{
    .Alloc = TestAlloc,
    .Free = TestFree,
    .Realloc = TestRealloc,
    .userdata = NULL
};


// =====================
// TEST SLAB
// =====================

static bool Test_SlabBasic(void)
{
    FusSlab_t* slab = FUSI_CreateSlab(
        &TestAllocation,
        4096,
        8,
        8,
        4096
    );

    FUS_TEST_ASSERT(
        slab != NULL,
        "Criacao do slab falhou"
    );


    int* value = FUSI_AllocSlab(
        slab,
        sizeof(int)
    );

    FUS_TEST_ASSERT(
        value != NULL,
        "Alloc int falhou"
    );


    *value = 12345;

    FUS_TEST_ASSERT(
        *value == 12345,
        "Valor escrito incorreto"
    );


    FUSI_FreeSlab(
        slab,
        value
    );


    FUSI_DestroySlab(slab);


    FUS_TEST_ASSERT(
        alloc_count == free_count,
        "Memory leak detectado"
    );


    return true;
}



// =====================
// MULTI ALLOC
// =====================

static bool Test_SlabMultipleAlloc(void)
{
    FusSlab_t* slab = FUSI_CreateSlab(
        &TestAllocation,
        8192,
        16,
        16,
        4096
    );


    FUS_TEST_ASSERT(
        slab != NULL,
        "Slab multi init falhou"
    );


    void* blocks[32];


    for (int i = 0; i < 32; i++)
    {
        blocks[i] = FUSI_AllocSlab(
            slab,
            64 + i
        );


        FUS_TEST_ASSERT(
            blocks[i] != NULL,
            "Alloc multiplo falhou"
        );


        memset(
            blocks[i],
            i,
            64 + i
        );
    }


    for (int i = 0; i < 32; i++)
    {
        unsigned char* data = blocks[i];

        FUS_TEST_ASSERT(
            data[0] == (unsigned char)i,
            "Memoria corrompida"
        );


        FUSI_FreeSlab(
            slab,
            blocks[i]
        );
    }


    FUSI_DestroySlab(slab);


    FUS_TEST_ASSERT(
        alloc_count == free_count,
        "Leak apos multiplas alocacoes"
    );


    return true;
}



// =====================
// REUSE TEST
// =====================

static bool Test_SlabReuse(void)
{
    FusSlab_t* slab = FUSI_CreateSlab(
        &TestAllocation,
        4096,
        8,
        8,
        4096
    );


    FUS_TEST_ASSERT(
        slab != NULL,
        "Slab reuse falhou"
    );


    void* a = FUSI_AllocSlab(
        slab,
        128
    );


    FUS_TEST_ASSERT(
        a != NULL,
        "Primeiro alloc falhou"
    );


    FUSI_FreeSlab(
        slab,
        a
    );


    void* b = FUSI_AllocSlab(
        slab,
        128
    );


    FUS_TEST_ASSERT(
        b != NULL,
        "Reuso falhou"
    );


    FUSI_FreeSlab(
        slab,
        b
    );


    FUSI_DestroySlab(slab);


    FUS_TEST_ASSERT(
        alloc_count == free_count,
        "Leak no reuse"
    );


    return true;
}

// =====================
// MAIN
// =====================

int main(void)
{
    printf("=== Fusion Slab Tests ===\n");


    if (!Test_SlabBasic())
        return 1;


    if (!Test_SlabMultipleAlloc())
        return 1;


    if (!Test_SlabReuse())
        return 1;


    printf("✅ Todos testes passaram!\n");


    printf(
        "Alloc: %zu Free: %zu\n",
        alloc_count,
        free_count
    );


    return 0;
}