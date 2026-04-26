#ifndef FUSION_IO_FILE_H
#define FUSION_IO_FILE_H

typedef struct FusIOBackend FusIOBackend_t;
void FUS_DestroyIOBackend(FusIOBackend_t* ctx);

FusIOBackend_t* FUS_IOBackendFile(const char* path);
#endif