#ifndef FUSION_COMPILE_H
#define FUSION_COMPILE_H
#include <Fusion/FusionTypes.h>
#include <Fusion/Backend/FusionBackend.h>
#include <Fusion/FusionRule.h>

/*
 * @brief Mount Bytes Of Mir
 * @param FusBufferContext_t* Buffer Acess
 * @param FusMirNode_t* Mir Node
 * @return FusStatusFlag_t Build Flag
*/
FusStatusFlag_t fusMountHidrsBytes(FusInstance instance,FusCommandRuleBase_t* compiler_rule,FusBackendReturn* out);
FusBufferContext_t* fusGetStreamBufferCompiler(FusInstance instance, FusBackendReturn ctx_backend);
void fusDestroyBackendReturn(FusInstance instance, FusBackendReturn ctx_backend);

#endif