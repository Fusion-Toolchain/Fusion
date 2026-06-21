#ifndef FUSION_PUBLIC_CODEMOUNT_H
#define FUSION_PUBLIC_CODEMOUNT_H
#include <Fusion/FusionTypes.h>
#include "HidrType.h"

typedef struct FusCodeMount_T* FusCodeMount_t;;

FusStatusFlag_t FUS_CreateCodeMount(FusInstance* ctx, FusCodeMount_t* out);

FusStatusFlag_t FUS_NewCallReg(FusCodeMount_t mount, int src_reg);
FusStatusFlag_t FUS_NewRet(FusCodeMount_t mount);
FusStatusFlag_t FUS_NewAddrMem(FusCodeMount_t mount,int base_reg, int offset, int dst_reg);
FusStatusFlag_t FUS_NewMov(FusCodeMount_t mount, FusHidrOperand_t dst, FusHidrOperand_t src);

size_t FUS_GetCountCode(FusCodeMount_t mount);
FusHidrNode_t* FUS_GetArryCode(FusCodeMount_t mount);
void FUS_DestroyCodeMount(FusInstance* ctx, FusCodeMount_t code);

#endif