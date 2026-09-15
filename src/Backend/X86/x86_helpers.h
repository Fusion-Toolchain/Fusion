#ifndef X86_INTERNAL_HELPERS_H
#define X86_INTERNAL_HELPERS_H
#include <Fusion/IRTypes/HidrType.h>
#include <Internal/IRTypes/Fus_HidrRegistre.h>
#include <Internal/Backend/Fus_Backend.h>

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

static inline size_t X86_MapVirtualReg(const HidrRegistre reg)
{
    static char buffer[25] = {0};
    if (reg.raw == HIDR_REGISTRE_INVALID) return (size_t)-1;

    uint8_t group = reg.desc.group;
    uint8_t role  = reg.desc.role;
    uint8_t index = reg.desc.index;

    if (group == HIDR_REGISTRE_GROUP_GP) {
        switch (role) {
            case HIDR_REGISTRE_ROLE_ACC:
            case HIDR_REGISTRE_ROLE_COUNTER:
            case HIDR_REGISTRE_ROLE_DATA:
            case HIDR_REGISTRE_ROLE_BASE:
            case HIDR_REGISTRE_ROLE_SP:
            case HIDR_REGISTRE_ROLE_BP:
            case HIDR_REGISTRE_ROLE_SRC:
            case HIDR_REGISTRE_ROLE_DST:
                if (index != 0) goto not_suport;
                if (role == HIDR_REGISTRE_ROLE_ACC) return X86_REG_RAX;
                if (role == HIDR_REGISTRE_ROLE_COUNTER) return X86_REG_RCX;
                if (role == HIDR_REGISTRE_ROLE_DATA) return X86_REG_RDX;
                if (role == HIDR_REGISTRE_ROLE_BASE) return X86_REG_RBX;
                if (role == HIDR_REGISTRE_ROLE_SP) return X86_REG_RSP;
                if (role == HIDR_REGISTRE_ROLE_BP) return X86_REG_RBP;
                if (role == HIDR_REGISTRE_ROLE_SRC) return X86_REG_RSI;
                return X86_REG_RDI;
            case HIDR_REGISTRE_ROLE_ARG:
            case HIDR_REGISTRE_ROLE_TMP:
                if (index < 8) return X86_REG_R8 + index;
                goto not_suport;
            default: goto not_suport;;
        }
    }

    if (group == HIDR_REGISTRE_GROUP_SIMD) {
        if (index < 16) return index; // xmm0-xmm15
        goto not_suport;
    }
    if (group == HIDR_REGISTRE_GROUP_SEG) {
        if (index < 6) return index;
        goto not_suport;
    }

not_suport:
    fusiRegistreToString(reg,buffer,sizeof(buffer));
    printf("Does not suport this %s\n",buffer);
    return (size_t)-1;
}

#endif