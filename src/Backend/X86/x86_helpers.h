#ifndef X86_INTERNAL_HELPERS_H
#define X86_INTERNAL_HELPERS_H
#include <Fusion/IRTypes/HidrType.h>
#include <Internal/Fus_Backend.h>

//HELPER
#include <Internal/Helpers/Fus_Helper_Codebase.h>

#include <stddef.h>
#include <stdio.h>
#include "x86_types.h"

static inline size_t X86_CalMirImmSize(FusHidrImmSize_t size_enum)
{
    switch (size_enum) {
        case HIDR_IMM8: return 1;
        case HIDR_IMM16: return 2;
        case HIDR_IMM32: return 4;
        case HIDR_IMM64: return 8;

        default: return 0;
    }
}

// TODO: Pre Implementação da janela de Virtual Registres.
static const size_t vreg_to_x86[] = {
    X86_REG_RAX, // V0
    X86_REG_RBX, // V1
    X86_REG_RCX, // V2
    X86_REG_RDX, // V3
    X86_REG_RSI, // V4
    X86_REG_RDI, // V5
};
#define X86_VREG_COUNT (sizeof(vreg_to_x86) / sizeof(vreg_to_x86[0]))
static inline size_t X86_MapVirtualReg(FusHidrVirtualReg_t reg)
{
    if (FUS_IS_SPECIAL(reg)) {
        switch (reg) {
            case HIDR_REG_STACK_PTR: return X86_REG_RSP;
            case HIDR_REG_BASE_PTR:  return X86_REG_RBP;
            default: return (size_t)-1;
        }
    }

    if (unlikely(reg >= (FusHidrVirtualReg_t)X86_VREG_COUNT)) return (size_t)-1;
    return vreg_to_x86[reg];
}

bool X86RegistreRealloc(FusBackendGenereteDataBlock_t* block, const char* name, FusBackendRealocOpaqueType_t type, size_t offset);

#endif