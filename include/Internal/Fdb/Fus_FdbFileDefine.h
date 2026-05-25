#ifndef FDB_FILE_MOUNT_H
#define FDB_FILE_MOUNT_H
#include <stdint.h>

#define FUSION_FDB_MAGIC 0x46444200  // "FDB\0"
typedef uint32_t FusFdbFileMagic_t;

typedef uint8_t FusFdbFileMode_t;
enum {
    FUSION_FDB_MODE_TYPE_EXECUTABLE = 0,
    FUSION_FDB_MODE_TYPE_RELOCATABLE = 1,
};
typedef uint16_t FusFdbFileVersion_t;
enum {
    FUSION_FDB_VERSION_1_0 = 0
};
typedef uint16_t FusFdbFileSectionFlag_t;
enum {
    FUSION_FDB_SECTION_FLAGS_TYPE_READ =   (1 << 0),
    FUSION_FDB_SECTION_FLAGS_TYPE_WRITE =  (1 << 1),
    FUSION_FDB_SECTION_FLAGS_TYPE_EXEC =   (1 << 2),
};
typedef uint8_t FusFdbFileSectionKind_t;
enum {
    FUSION_FDB_SECTION_KIND_TYPE_CODE = 1,
    FUSION_FDB_SECTION_KIND_TYPE_DATA = 2,
    FUSION_FDB_SECTION_KIND_TYPE_RODATA = 3,
    FUSION_FDB_SECTION_KIND_TYPE_BSS = 4,
    FUSION_FDB_SECTION_KIND_TYPE_CUSTOM = 5,
};


typedef struct __attribute__((packed)) {
    FusFdbFileMagic_t magic;
    FusFdbFileVersion_t version;
    FusFdbFileMode_t mode;
    uint64_t file_size;

    uint8_t endianness;

    char target_triple[32];

    uint16_t section_count;
    uint16_t symbol_count;
    uint16_t reloc_count;

    uint64_t string_table_offset; // STRING
    uint32_t string_table_size;

    uint64_t section_table_offset;
    uint64_t symbol_table_offset;
    uint64_t reloc_table_offset;

    uint64_t entry_section_index; // EXECUTABLE
    uint64_t entry_point; // EXECUTABLE
} FusFdbFileHeader_t;

typedef struct __attribute__((packed)) {
    uint32_t name_offset;

    uint32_t offset;
    uint32_t size; // FILE SIZE
    uint32_t vsize; // MEMORY SIZE

    FusFdbFileSectionFlag_t flags;
    FusFdbFileSectionKind_t kind;

    uint32_t alignment;
} FusFdbFileSection_t;

#endif