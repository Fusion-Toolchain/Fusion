#include <Fusion/IRTypes/HidrType.h>
#include <Internal/IRTypes/Fus_CodeBuffer.h>
#include <Internal/Fus_Instance.h>

// HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>
#include <Internal/Helpers/Fus_Helper_Allocation.h>

// TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

FusStatusFlag_t fusCreateCodeMount(FusInstance* ctx, FusCodeMount* out)
{
    if (unlikely(!ctx || !out)) return FUSION_ERRO;
    struct FusInstance_T* instance_real = *ctx;

    *out = NULL;

    struct FusCodeMount_T* code_ctx = FUSIH_ALLOC(instance_real->allocation, sizeof(struct FusCodeMount_T));
    if (unlikely(!code_ctx)) return FUSION_ERRO;

    code_ctx->code_capacity = 10;

    size_t init_size = code_ctx->code_capacity * sizeof(FusHidrNode_t);
    FusHidrNode_t* code_hidr = FUSIH_ALLOC(instance_real->allocation,init_size);
    if (unlikely(!code_hidr)) {
        FUSIH_FREE(instance_real->allocation,code_ctx);
        return FUSION_ERRO;
    }

    code_ctx->code_arry = code_hidr;
    code_ctx->code_count = 0;

    code_ctx->ref_ctx = instance_real;

    *out = code_ctx;
    return FUSION_OK;
}

static inline bool ExpansiveCodeBuffer(struct FusCodeMount_T* mount)
{
    if (unlikely(!mount)) return false;
    struct FusInstance_T* ctx = mount->ref_ctx;

    if (mount->code_count + 1 > mount->code_capacity) {
        size_t new_capacity = mount->code_capacity * 2;
        size_t new_size = new_capacity * sizeof(FusHidrNode_t);

        FusHidrNode_t* new_arry = FUSIH_REALLOC(ctx->allocation,mount->code_arry,new_size);
        if (unlikely(!new_arry)) return false;

        mount->code_arry = new_arry;
        mount->code_capacity = new_capacity;
    }

    return true;
}
FusStatusFlag_t fusInsertCodeBlock(struct FusCodeMount_T* mount, FusHidrNode_t node)
{
    if (unlikely(!mount)) return FUSION_ERRO;
    if (unlikely(!ExpansiveCodeBuffer(mount))) return FUSION_ERRO;

    mount->code_arry[mount->code_count] = node;
    mount->code_count++;

    return FUSION_OK;
}

void fusDestroyCodeMount(FusInstance ctx, FusCodeMount code)
{
    if (unlikely(!ctx || !code)) return;

    FUSIH_FREE(ctx->allocation,code->code_arry);
    FUSIH_FREE(ctx->allocation,code);

    code = NULL;
}