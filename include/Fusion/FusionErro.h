#ifndef FUSION_PUBLIC_TRACED_H
#define FUSION_PUBLIC_TRACED_H
#include <stddef.h>

typedef enum {
    FUS_TRACED_TYPE_NONE = 0,

    FUS_TRACED_TYPE_OK,
    FUS_TRACED_TYPE_ERRO,
    FUS_TRACED_TYPE_WARN,
    FUS_TRACED_TYPE_CRITCAL,
} FusTracedErroType_t;
typedef enum {
    FUS_TRACED_LOCAL_NONE = 0,

    FUS_TRACED_LOCAL_LINKER,
    FUS_TRACED_LOCAL_COMPILER,
    FUS_TRACED_LOCAL_SAVED,
    FUS_TRACED_LOCAL_CHECK,
} FusTracedErroLocal_t;
typedef struct FusTracedErro FusTracedErro_t;

FusTracedErro_t* FUS_CreateTracedErro();
void FUS_DestroyTracedErro(FusTracedErro_t* ctx);
FusTracedErroType_t FUS_GetTracedErroType(FusTracedErro_t* ctx);
FusTracedErroLocal_t FUS_GetTracedErroLocal(FusTracedErro_t* ctx);
const char* FUS_GetTracedErroMsg(FusTracedErro_t* ctx);

#endif