#ifndef FUSION_FILE_INTERFACE_H
#define FUSION_FILE_INTERFACE_H

#include <Fusion/FusionTypes.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FUS_FILE_SECTION_TYPE_READ  (1 << 0)
#define FUS_FILE_SECTION_TYPE_WRITE (1 << 1)
#define FUS_FILE_SECTION_TYPE_EXEC  (1 << 2)
typedef uint32_t FusFileManagerSectionType_t;

typedef struct {
    const char* name;

    size_t section_index;
    size_t offset;
    bool is_global;
} FusFileManagerSymbol_t;

typedef enum {
    FUS_FILE_RELOC_TYPE_REL32,
    FUS_FILE_RELOC_TYPE_ABS64,
} FusFileManagerRelocType_t;
typedef struct {
    FusFileManagerSymbol_t* symbol;
    FusFileManagerRelocType_t type;

    size_t section_index;
    size_t offset;
} FusFileManagerReloc_t;

/*
 * @brief Section Define
*/
typedef struct {
    const char* name;
    size_t size;
    size_t offset;
    size_t alignment;
    FusFileManagerSectionType_t type;
} FusFileManagerSectionDefine_t;
typedef struct {
    const char* name;
    FusFileManagerSectionType_t type;

    size_t offset;
    size_t size;
    size_t alignment;
} FusFileManagerSection_t;
typedef struct {
    FusFileManagerSection_t* sections;
    size_t sections_count;
    FusFileManagerSymbol_t* symbols;
    size_t symbols_count;
    FusFileManagerReloc_t* realocs;
    size_t realocs_count;

    struct {
        const char* name;
        int file_fd;
    } file;
    struct {
        uint8_t* file_map;
        size_t file_size;
    } file_access;
} FusFileManager_t;

FusFileManager_t* FUS_CreateFileDevice(const char* name);
void FUS_FileSectionAdd(FusFileManager_t* ctx,FusFileManagerSectionDefine_t* define);
FusStatusFlag_t FUS_SaveFileDeviceForBuffer(FusFileManager_t* ctx, FusBufferContext_t* buffer);
void FUS_DestroyFileDevice(FusFileManager_t* ctx);
#endif