#include <Fusion/FusionFile.h>
#include <Fusion/FusionTypes.h>

#include <Internal/Memory/Fus_Arena.h>
#include <Internal/Memory/Fus_Handle.h>

#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define FUSION_FILE_MAX_SECTIONS 200

FusFileManager_t* FUS_CreateFileDevice(const char* name)
{
    FusFileManager_t* ctx = malloc(sizeof(FusFileManager_t));
    if (!ctx) return NULL;

    int fd = open(name,O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        free(ctx);
        return NULL;
    }
    ctx->file.name = strdup(name);
    if (!ctx->file.name) {
        close(fd);
        free(ctx);
        return NULL;
    }
    FusFileManagerSection_t* sections = malloc(sizeof(FusFileManagerSection_t)*FUSION_FILE_MAX_SECTIONS);
    if (!sections) {
        close(fd);
        free(ctx);
        free((char*)ctx->file.name);
        return NULL;
    }

    ctx->file.file_fd = fd;
    
    ctx->realocs = NULL;
    ctx->realocs_count = 0;
    
    ctx->symbols = NULL;
    ctx->symbols_count = 0;

    ctx->sections = sections;
    ctx->sections_count = 0;


    return ctx;
}

void FUS_FileSectionAdd(FusFileManager_t* ctx,FusFileManagerSectionDefine_t* define)
{
    if (!define) return;
    if (!define->name || define->size == 0) return;

    FusFileManagerSection_t* section = &ctx->sections[ctx->sections_count];
    section->size = define->size;
    section->alignment = define->alignment;
    section->name = define->name;
    section->type = define->type;

    ctx->sections_count++;
}

FusStatusFlag_t FUS_SaveFileDeviceForBuffer(FusFileManager_t* ctx, FusBufferContext_t* buffer)
{
    if (!ctx || !buffer) return FUSION_ERRO;

    ssize_t written = write(ctx->file.file_fd,buffer->buffer,buffer->offset);
    if (written < 0 || written != buffer->offset) return FUSION_ERRO;

    return FUSION_OK;
}

void FUS_DestroyFileDevice(FusFileManager_t* ctx)
{
    if (!ctx) return;
    close(ctx->file.file_fd);

    if (ctx->realocs) free(ctx->realocs);
    if (ctx->symbols) free(ctx->symbols);
    if (ctx->sections) free(ctx->sections);

    free((char*)ctx->file.name);
    free(ctx);
}