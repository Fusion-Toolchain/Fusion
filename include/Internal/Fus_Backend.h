#ifndef FUSION_INTERNAL_BACKEND_H
#define FUSION_INTERNAL_BACKEND_H
#include <Internal/Memory/Fus_Arena.h>

#include <Fusion/IRTypes/HidrType.h>
#include <Fusion/Backend/FusionBackend.h>
#include <Fusion/FusionTypes.h>

#include <stddef.h>
#include <stdint.h>


typedef struct {
    uint8_t* buffer;          // buffer de bytes pra patchear
    size_t    offset;                   // onde patchear
    uint64_t  sym_addr;                // endereço do símbolo
    uint64_t  patch_addr;             // endereço do próprio patch (pra REL32)
} FusBackendRelocContext_t;
typedef uint32_t FusBackendRealocOpaqueType_t;

typedef struct {
    const char* name;
    FusBackendRealocOpaqueType_t type;
    size_t offset;
} FusBackendReallocNeed_t;

typedef struct {
    const void* data;
    void (*free)(const void* data);
} FusBackendTrasferLifeTime_t;

typedef struct FusBackendGenereteDataBlock FusBackendGenereteDataBlock_t; // REF

typedef struct FusBackendApi {
    void* (*FusAlloc)(struct FusBackendApi*,size_t);
    void  (*FusFree)(struct FusBackendApi*, void*);
    FusBackendTrasferLifeTime_t* (*FusCreateTrasfer)(
        struct FusBackendApi*, void* data,void (*free)(const void* data)
    );
    void (*FusDestroyTrasfer)(
        struct FusBackendApi*, FusBackendTrasferLifeTime_t*
    );
    FusBackendGenereteDataBlock_t* (*FusCreateDataBlock)(struct FusBackendApi*, size_t, size_t);
    void (*FusDestroyDataBlock)(struct FusBackendApi*, FusBackendGenereteDataBlock_t*);

    FusInstance* Instance;
} FusBackendApi_t;
typedef struct FusBackendGenereteDataBlock {
    //FusBufferContext_t* buffer;
    uint8_t* buffer_slab;
    size_t slab_size;
    size_t slab_offset;

    FusMemoryArena_t* arena;
    FusBackendReallocNeed_t* realoc;
    size_t realoc_count;
    size_t realoc_capacity;

    FusStatusFlag_t flag;
    FusBackendApi_t* api;
} FusBackendGenereteDataBlock_t;

typedef struct {
    FusBackendTrasferLifeTime_t* (*FUSI_BackendMountHidr)(const FusHidrNode_t*);
    FusBackendTrasferLifeTime_t* (*FUSI_BackendMountHidrArry)(const FusHidrNode_t*,size_t);
    FusStatusFlag_t (*FUSI_BackendLinkerRealloc)(FusBackendRealocOpaqueType_t,FusBackendRelocContext_t*);
} FusBackendInterface_t;

struct FusBackendReturn {
    FusBackendTrasferLifeTime_t* transfer_data;
    FusBackendApi_t* api;
};
struct FusModuleBackend_T {
    FusModuleBackendType_t type;
    const char* name;
    FusBackendInterface_t* interface; // INTERFACE INTERNA
    FusBackendApi_t* api; // ONCE FOR MODULE
};
typedef FusBackendInterface_t* (*FusBackendInterfaceDefine_t)(FusBackendApi_t*);

#endif