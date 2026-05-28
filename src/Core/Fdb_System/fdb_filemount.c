#include <Internal/Fdb/Fus_FdbFileDefine.h>
#include <Fusion/FusionTypes.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    uint8_t* buffer;
    size_t size;
    size_t offset;
} FDBWriter_t;
static inline void* FDBCollectSpace(FDBWriter_t* w, size_t size)
{
    if (w->offset + size > w->size) return NULL;

    void* ptr = (void*)(w->buffer + w->offset);
    w->offset += size;
    return ptr;
}


static FusStatusFlag_t FDBFile_MountHeader(FDBWriter_t* buffer)
{
    if (!buffer) return FUSION_ERRO;

    FusFdbFileHeader_t* header = FDBCollectSpace(buffer, sizeof(FusFdbFileHeader_t));
    if (!header) return FUSION_ERRO;

    header->magic = FUSION_FDB_MAGIC;
    header->version = FUSION_FDB_VERSION_1_0;
    header->mode = FUSION_FDB_MODE_TYPE_RELOCATABLE;
    header->file_size = 4*1024;
    header->endianness = 4;

    strcpy(header->target_triple, "x86-fdb");

    return FUSION_OK;
}

static inline FusStatusFlag_t WriteAll(int fd,FDBWriter_t* buffer)
{
    ssize_t total = 0;
    while ((size_t)total < buffer->offset) {
        ssize_t w = write(fd, buffer->buffer + total, buffer->offset - total);
        if (w <= 0) return FUSION_ERRO;
        total += w;
    }
    return FUSION_OK;
}
FusStatusFlag_t FDBFile_MountFDB(const char* path, size_t sections_count)
{
    if (!path || sections_count == 0) return FUSION_ERRO;

    int fd = open(path,O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0) return FUSION_ERRO;

    size_t size = sizeof(FusFdbFileHeader_t)+(sizeof(FusFdbFileSection_t)*sections_count);
    void* buffer_data = malloc(size);
    if (!buffer_data) {
        close(fd);
        return FUSION_ERRO;
    }

    FDBWriter_t buffer = {
        .offset = 0,
        .size = size,
        .buffer = buffer_data
    };

    if (FDBFile_MountHeader(&buffer) != FUSION_OK) {
        close(fd);
        return FUSION_ERRO;
    }

    if (WriteAll(fd,&buffer) != FUSION_OK) {
        close(fd);
        return FUSION_ERRO;
    }
    free(buffer_data);
    return FUSION_OK;
}