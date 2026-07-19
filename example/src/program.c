#include <Fusion/Fusion.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

void MountCode(FusCodeMount Mount)
{
    FUS_InsertCodeBlock(Mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(0),
            FUS_HIDR_Imm(60,HIDR_IMM64))
    );
    FUS_InsertCodeBlock(Mount,
        FUS_HIDRM(HIDR_INSTR_MOV,
            HIDR_OP_SIZE_64,
            FUS_HIDR_Reg(5),
            FUS_HIDR_Imm(1,HIDR_IMM64))
    );
    FUS_InsertCodeBlock(Mount,
        FUS_HIDRM(HIDR_INSTR_SYSCALL,
            HIDR_OP_SIZE_NONE,
            FUS_HIDR_None(),
            FUS_HIDR_None())
    );
}

int WriteExecutableELF(const char *filename,const uint8_t *code,size_t code_size)
{
    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0755);
    if (fd < 0) return 0;
    Elf64_Ehdr ehdr = {0};

    memcpy(ehdr.e_ident, ELFMAG, SELFMAG);
    ehdr.e_ident[EI_CLASS]   = ELFCLASS64;
    ehdr.e_ident[EI_DATA]    = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_ident[EI_OSABI]   = ELFOSABI_SYSV;

    ehdr.e_type      = ET_EXEC;
    ehdr.e_machine   = EM_X86_64;
    ehdr.e_version   = EV_CURRENT;
    ehdr.e_entry     = 0x401000;
    ehdr.e_phoff     = sizeof(Elf64_Ehdr);
    ehdr.e_ehsize    = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum     = 1;

    Elf64_Phdr phdr = {0};

    phdr.p_type   = PT_LOAD;
    phdr.p_offset = 0;
    phdr.p_vaddr  = 0x400000;
    phdr.p_paddr  = 0x400000;
    phdr.p_filesz = 0x1000 + code_size;
    phdr.p_memsz  = 0x1000 + code_size;
    phdr.p_flags  = PF_R | PF_X;
    phdr.p_align  = 0x1000;

    write(fd, &ehdr, sizeof(ehdr));
    write(fd, &phdr, sizeof(phdr));

    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos < 0) {
        close(fd);
        return 0;
    }
    char zero = 0;
    while (pos < 0x1000) {
        write(fd, &zero, 1);
        pos++;
    }
    write(fd, code, code_size);

    close(fd);
    return 1;
}

int main(void)
{
    FusInstance Instance = NULL;
    FusCodeMount CodeMount = NULL;

    if (!FUS_CreateInstance(&Instance, NULL)) {
        printf("Erro Init Instance!\n");
        return 1;
    }

    if (!FUS_CreateCodeMount(&Instance, &CodeMount)) {
        printf("Erro Init CodeMount System\n");
        return 1;
    }
    MountCode(CodeMount);

    FusModuleBackend BackendInstance = NULL;
    if (!FUS_LoaderBackend(Instance, &BackendInstance, "X86_Backend", FUS_BACKEND_TYPE_STATIC)) {
        printf("Erro Init BackendModule\n");
        return 1;
    }

    FusCommandBackend BackendChain = {
        .sType = FUS_COMMAND_SEND_BACKEND,
        .backend = BackendInstance,
        .pNext = NULL
    };
    FusCommandHidr HidrChain = {
        .sType = FUS_COMMAND_SEND_HIDR,
        .code = CodeMount,
        .pNext = (FusCommandRuleBase_t*)&BackendChain
    };

    FusBackendReturn BackendReturn = NULL;
    if (!FUS_MountHidrsBytes(Instance, (FusCommandRuleBase_t*)&HidrChain, &BackendReturn)) {
        printf("Erro Compile Code\n");
        return 1;
    }
    FusBufferContext_t* CodeBuffer = FUS_GetStreamBufferCompiler(BackendReturn);
    if (!CodeBuffer) {
        printf("Erro obter buffer compilado\n");
        return 1;
    }

    if (!WriteExecutableELF("output.elf", CodeBuffer->buffer, CodeBuffer->offset)) {
        printf("Erro escrever ELF\n");
        return 1;
    }
    printf("Execute: ./output.elf\n echo $?\n");

    FUS_DestroyBufferCode(CodeBuffer);
    FUS_DestroyBackendReturn(Instance, BackendReturn);
    FUS_DestroyBackend(BackendInstance);
    FUS_DestroyCodeMount(Instance, CodeMount);
    FUS_DestroyInstance(Instance);

    return 0;
}