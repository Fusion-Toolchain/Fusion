#ifndef FUSION_INTERNAL_BACKEND_H
#define FUSION_INTERNAL_BACKEND_H
#include <Internal/Memory/Fus_Arena.h>

#include <Fusion/FusionErro.h>
#include <Fusion/IRTypes/HidrType.h>
#include <Fusion/Backend/FusionBackend.h>
#include <Fusion/FusionTypes.h>

#include <stddef.h>
#include <stdint.h>

typedef struct {
    void* (*FusAlloc)(size_t);
    void  (*FusFree)(void*);
} FusBackendApi_t;

typedef struct {
    FusBufferContext_t* buffer;          // buffer de bytes pra patchear
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
    FusBufferContext_t* buffer;

    FusMemoryArena_t* arena;
    FusBackendReallocNeed_t* realoc;
    size_t realoc_count;
    size_t realoc_capacity;

    FusStatusFlag_t flag;
} FusBackendGenereteDataBlock_t;
typedef struct {
    const void* data;
    void (*free)(const void* data);
} FusBackendTrasferLifeTime_t;

typedef struct {
    FusBackendTrasferLifeTime_t* (*FUSI_BackendMountHidr)(FusTracedErro_t*,const FusHidrNode_t*);
    FusBackendTrasferLifeTime_t* (*FUSI_BackendMountHidrArry)(FusTracedErro_t*,const FusHidrNode_t*,size_t);
    FusStatusFlag_t (*FUSI_BackendLinkerRealloc)(FusTracedErro_t*,FusBackendRealocOpaqueType_t,FusBackendRelocContext_t*);
} FusBackendInterface_t;

struct FusBackendReturn {
    FusBackendTrasferLifeTime_t* transfer_data;
};
struct FusModuleBackend {
    FusModuleBackendType_t type;
    const char* name;
    FusBackendInterface_t* interface; // INTERFACE INTERNA
    FusBackendApi_t* api; // ONCE FOR MODULE
};
#endif