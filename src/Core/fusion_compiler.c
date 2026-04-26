#include <Fusion/Fusion.h>

#include <Internal/Fus_Error.h>
#include <Internal/Fus_Backend.h>

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

static inline bool _ArgumentProcessMountHidrBytes(
    FusCommandRuleBase_t*  compiler_rule,
    FusModuleBackend_t** backend,
    FusHidrNode_t**        hidr,
    size_t*                count,
    FusTracedErro_t**      trace
)
{
    if (!compiler_rule) return false;

    FusCommandRuleBase_t* node = compiler_rule;
    while (node) {
        switch (node->sType) {
            case FUS_COMMAND_SEND_BACKEND: {
                FusCommandBackend* b = (FusCommandBackend*)node;
                *backend = b->backend;
                break;
            }
            case FUS_COMMAND_SEND_HIDR: {
                FusCommandHidr* r = (FusCommandHidr*)node;
                *hidr  = r->hidr_arry;
                *count = r->hidr_count;
                break;
            }
            case FUS_COMMAND_SEND_TRACE: {
                *trace = ((FusCommandTraceContext*)node)->trace_data;
                break;
            }
            default: return false;
        }
        node = (FusCommandRuleBase_t*)node->pNext;
    }
    return true;
}

FusBackendReturn_t* FUS_MountHidrsBytes(FusCommandRuleBase_t* compiler_rule)
{
    if (!compiler_rule) return NULL;

    FusBackendReturn_t* ctx = malloc(sizeof(FusBackendReturn_t));
    if (!ctx) return NULL;

    FusModuleBackend_t* backend = NULL;
    FusHidrNode_t* hidr = NULL;
    size_t count = 0;
    FusTracedErro_t* trace = NULL;

    if(!_ArgumentProcessMountHidrBytes(compiler_rule,&backend,&hidr,&count,&trace)) return NULL;
    if (!backend || !hidr || !count) return NULL;
    if (count == 0) return NULL;

    FusBackendInterface_t* interface = backend->interface;
    FusBackendApi_t* API = backend->api;
    FusBackendTrasferLifeTime_t* backend_data = interface->FUSI_BackendMountHidrArry(trace,hidr,count);
    if (!backend_data) return NULL;

    ctx->transfer_data = backend_data;

    return ctx;
}