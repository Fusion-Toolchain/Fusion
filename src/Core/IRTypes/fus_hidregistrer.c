/**
 * ███████╗██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
 * ██╔════╝██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
 * █████╗  ██║   ██║███████╗██║██║   ██║██╔██╗ ██║
 * ██╔══╝  ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
 * ██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 * ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
 *
 * @file    fus_hidregistrer.c
 * @brief   Conversion of names to IR registers.
 * @author     Ewerton23929dev
 *
 * @details
 * Parses the name case insensitively, validates the group and returns the
 * register identifier, or the invalid value when the name does not exist.
 * @copyright  Copyright (c) 2026 Ewerton23929dev. All rights reserved.
 */

#include <Internal/IRTypes/Fus_HidrRegistre.h>
#include <Fusion/FusionTypes.h>

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

static inline int CharEqualCi(char a, char b)
{
    return tolower((unsigned char)a) == tolower((unsigned char)b);
}
HidrRegistre fusiConvertStringToRegistre(const char* registre)
{
    if (!registre) return (HidrRegistre){0};
    HidrRegDesc desc = {0};
    bool group_define   = false;
    bool purpose_define = false;
    bool registre_size  = false;

    while (*registre != '\0') {
        char represent = *registre;

        // ROLE
        if (!purpose_define) {
            if      (CharEqualCi(represent, 'A')) { desc.role = HIDR_REGISTRE_ROLE_ACC;     purpose_define = true; }
            else if (CharEqualCi(represent, 'B')) { desc.role = HIDR_REGISTRE_ROLE_BASE;    purpose_define = true; }
            else if (CharEqualCi(represent, 'T')) { desc.role = HIDR_REGISTRE_ROLE_COUNTER; purpose_define = true; }
            else if (CharEqualCi(represent, 'D')) { desc.role = HIDR_REGISTRE_ROLE_DATA;    purpose_define = true; }
            else if (CharEqualCi(represent, 'S')) { desc.role = HIDR_REGISTRE_ROLE_SP;      purpose_define = true; }
            else if (CharEqualCi(represent, 'P')) { desc.role = HIDR_REGISTRE_ROLE_BP;      purpose_define = true; }
            else if (CharEqualCi(represent, 'I')) { desc.role = HIDR_REGISTRE_ROLE_SRC;     purpose_define = true; }
            else if (CharEqualCi(represent, 'O')) { desc.role = HIDR_REGISTRE_ROLE_DST;     purpose_define = true; }
            else if (CharEqualCi(represent, 'K')) { desc.role = HIDR_REGISTRE_ROLE_LINK;    purpose_define = true; }
            else if (CharEqualCi(represent, 'R')) { desc.role = HIDR_REGISTRE_ROLE_RET;     purpose_define = true; }
            else if (CharEqualCi(represent, 'G')) { desc.role = HIDR_REGISTRE_ROLE_ARG;     purpose_define = true; }
            else if (CharEqualCi(represent, 'M')) { desc.role = HIDR_REGISTRE_ROLE_TMP;     purpose_define = true; }
        }

        // GROUP
        if (!group_define) {
            if      (CharEqualCi(represent, 'C')) { desc.group = HIDR_REGISTRE_GROUP_GP;    group_define = true; }
            else if (CharEqualCi(represent, 'F')) { desc.group = HIDR_REGISTRE_GROUP_FLOAT; group_define = true; }
            else if (CharEqualCi(represent, 'V')) { desc.group = HIDR_REGISTRE_GROUP_SIMD;  group_define = true; }
            else if (CharEqualCi(represent, 'X')) { desc.group = HIDR_REGISTRE_GROUP_CTRL;  group_define = true; }
            else if (CharEqualCi(represent, 'E')) { desc.group = HIDR_REGISTRE_GROUP_SEG;   group_define = true; }
            else if (CharEqualCi(represent, 'Z')) { desc.group = HIDR_REGISTRE_GROUP_DEBUG; group_define = true; }
        }

        // SIZE
        if (!registre_size) {
            if      (CharEqualCi(represent, 'N')) { desc.size = HIDR_REGISTRE_SIZE_8;   registre_size = true; }
            else if (CharEqualCi(represent, 'W')) { desc.size = HIDR_REGISTRE_SIZE_16;  registre_size = true; }
            else if (CharEqualCi(represent, 'H')) { desc.size = HIDR_REGISTRE_SIZE_32;  registre_size = true; }
            else if (CharEqualCi(represent, 'L')) { desc.size = HIDR_REGISTRE_SIZE_64;  registre_size = true; }
            else if (CharEqualCi(represent, 'Q')) { desc.size = HIDR_REGISTRE_SIZE_128; registre_size = true; }
            else if (CharEqualCi(represent, 'U')) { desc.size = HIDR_REGISTRE_SIZE_256; registre_size = true; }
            else if (CharEqualCi(represent, 'Y')) { desc.size = HIDR_REGISTRE_SIZE_512; registre_size = true; }
        }

        // INDEX
        if (isdigit((unsigned char)represent)) {
            uint32_t number = 0;
            while (isdigit((unsigned char)*registre)) {
                number = number * 10 + (*registre - '0');
                registre++;
            }
            desc.index = (uint8_t)number;
            continue;
        }

        registre++;
    }
    if (!group_define && !purpose_define && !registre_size)
        return (HidrRegistre){.raw = HIDR_REGISTRE_INVALID};
    // validação semântica: singletons devem ter index 0
    if (desc.group == HIDR_REGISTRE_GROUP_GP) {
        switch (desc.role) {
            case HIDR_REGISTRE_ROLE_ACC:
            case HIDR_REGISTRE_ROLE_BASE:
            case HIDR_REGISTRE_ROLE_COUNTER:
            case HIDR_REGISTRE_ROLE_DATA:
            case HIDR_REGISTRE_ROLE_SP:
            case HIDR_REGISTRE_ROLE_BP:
            case HIDR_REGISTRE_ROLE_SRC:
            case HIDR_REGISTRE_ROLE_DST:
                if (desc.index != 0) return (HidrRegistre){.raw = HIDR_REGISTRE_INVALID};
                break;
            case HIDR_REGISTRE_ROLE_ARG:
            case HIDR_REGISTRE_ROLE_TMP:
                if (desc.index >= 8) return (HidrRegistre){.raw = HIDR_REGISTRE_INVALID};
                break;
            default: break;
        }
    } else if (desc.group == HIDR_REGISTRE_GROUP_SIMD) {
        if (desc.index >= 16) return (HidrRegistre){.raw = HIDR_REGISTRE_INVALID};
    } else if (desc.group == HIDR_REGISTRE_GROUP_SEG) {
        if (desc.index >= 6) return (HidrRegistre){.raw = HIDR_REGISTRE_INVALID};
    }
    return (HidrRegistre){ .desc = desc };
}

void fusiRegistreToString(HidrRegistre reg, char* buf, size_t len)
{
    if (!buf || len == 0) return;
    size_t pos = 0;

    // ROLE
    char role_char = '?';
    switch (reg.desc.role) {
        case HIDR_REGISTRE_ROLE_ACC:     role_char = 'A'; break;
        case HIDR_REGISTRE_ROLE_BASE:    role_char = 'B'; break;
        case HIDR_REGISTRE_ROLE_COUNTER: role_char = 'T'; break;
        case HIDR_REGISTRE_ROLE_DATA:    role_char = 'D'; break;
        case HIDR_REGISTRE_ROLE_SP:      role_char = 'S'; break;
        case HIDR_REGISTRE_ROLE_BP:      role_char = 'P'; break;
        case HIDR_REGISTRE_ROLE_SRC:     role_char = 'I'; break;
        case HIDR_REGISTRE_ROLE_DST:     role_char = 'O'; break;
        case HIDR_REGISTRE_ROLE_LINK:    role_char = 'K'; break;
        case HIDR_REGISTRE_ROLE_RET:     role_char = 'R'; break;
        case HIDR_REGISTRE_ROLE_ARG:     role_char = 'G'; break;
        case HIDR_REGISTRE_ROLE_TMP:     role_char = 'M'; break;
    }
    if (pos < len) buf[pos++] = role_char;

    // GROUP
    char group_char = '?';
    switch (reg.desc.group) {
        case HIDR_REGISTRE_GROUP_GP:    group_char = 'C'; break;
        case HIDR_REGISTRE_GROUP_FLOAT: group_char = 'F'; break;
        case HIDR_REGISTRE_GROUP_SIMD:  group_char = 'V'; break;
        case HIDR_REGISTRE_GROUP_CTRL:  group_char = 'X'; break;
        case HIDR_REGISTRE_GROUP_SEG:   group_char = 'E'; break;
        case HIDR_REGISTRE_GROUP_DEBUG: group_char = 'Z'; break;
    }
    if (pos < len) buf[pos++] = group_char;

    // SIZE
    char size_char = '?';
    switch (reg.desc.size) {
        case HIDR_REGISTRE_SIZE_8:   size_char = 'N'; break;
        case HIDR_REGISTRE_SIZE_16:  size_char = 'W'; break;
        case HIDR_REGISTRE_SIZE_32:  size_char = 'H'; break;
        case HIDR_REGISTRE_SIZE_64:  size_char = 'L'; break;
        case HIDR_REGISTRE_SIZE_128: size_char = 'Q'; break;
        case HIDR_REGISTRE_SIZE_256: size_char = 'U'; break;
        case HIDR_REGISTRE_SIZE_512: size_char = 'Y'; break;
    }
    if (pos < len) buf[pos++] = size_char;

    // INDEX
    int written = snprintf(buf + pos, len - pos, "%u", (unsigned)reg.desc.index);
    if (written > 0) pos += written;

    if (pos < len) buf[pos] = '\0';
}

FusHidrRegistre fusInterpreterRegistre(char* string)
{
    if (!string) return HIDR_REGISTRE_INVALID;
    return fusiConvertStringToRegistre(string).raw;
}