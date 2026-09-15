#ifndef FUSION_HIDR_REGISTRE_H
#define FUSION_HIDR_REGISTRE_H
#include <stdint.h>

#define HIDR_REGISTRE_INVALID 0xFFFFFFFF
typedef uint32_t FusHidrRegistre;

FusHidrRegistre fusInterpreterRegistre(char* string);
#endif