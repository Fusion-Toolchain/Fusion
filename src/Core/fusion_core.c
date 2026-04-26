#include <Fusion/FusionTypes.h>

const char* FUS_StrError(FusStatusFlag_t status)
{
    switch(status) {
        case FUSION_OK: return "OK";
        case FUSION_ERRO: return "Generic error";
        default: return "Unknown FusionStatusFlag";
    }
}