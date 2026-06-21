# Fusion Handles System

Do podereso mundo de troca de contexto, fizemos esta arquitetura pensando e troca zero-copy de buffer e dados entre o Backend(ISA Define) e corpo central *Fusion Core*, sendo apenas uma troca simples de life-time.

![Show](img/FusionHandleShow.png)

## - Description

Baseado na forma de Kernel Unix de ser comportar o sistema funciona utilizando handles para troca de contexto, com apenas uma fonte real de dados centrais, ambos o sistemas alocam no mesmo serviço de memoria, mas cada um possui seu devido direito de life-time, onde o sistema de handles entra em jogo, para possibilitar a mundaça de novo entre ambos os *Consumidore/Produtores*.

### - Handle Sets

![Format](img/HandleStruct.png)

| Bits | Campo | Descrição |
|------|-------|------------|
| 24-31 | GENERATION (8 bits) | Incrementa cada vez que o slot é reusado. Detecta use-after-free. |
| 16-23 | TYPE (8 bits) | Identifica o tipo do objeto (ex: 1=backend, 2=buffer, 3=symbol). |
| 0-15 | ID (16 bits) | Índice na tabela de handles (0-65535). Acesso O(1). |

Formato de handle para suporta segurançã por geração, alem de indetidade de tipo, alem de seu *ID* para tabela de acesso O(1), sistema feito para ser performatico e rebusto.

### - Exemple Internal API

```c
#include <Internal/Memory/Fus_Handle.h>

FusTable_t table = NULL;

#define EXEMPLE_INT_TYPE 1

FusMemoryId_t ToMyExempleProduct(void)
{
    if (FUSI_InitHandleSystem(&table) != FUSION_OK) { // INIT SYSTEM
        return FUSION_INVALID_HANDLE;
    }

    static int a = 90;

    FusMemoryId_t myHandle = FUSI_AllocHandle(
        &table,
        &a,
        EXEMPLE_INT_TYPE,
        DestroyFunction
    );

    return handle;
}

void ToMyExempleConsumer(FusMemoryId_t handle)
{
    if (handle == FUSION_INVALID_HANDLE) {
        // Erro is invalid!
        return;
    }

    uint16_t id = FUSI_HandleGetId(handle);
    uint8_t type = FUSI_HandleGetType(handle);
    uint8_t geration = FUSI_HandleGetGen(handle);

    void* memory = FUSI_GetDataHandle(handle); // TRASFORM IN INVALID!
    FUSI_FreeHandle(handle); // EXCLUDE FOR RE-USAGE

    FUSI_CloseHandleSystem(&table); // CLOSE SYSTEM
}

```

## - Review Files

- **Core/Memory/fus_handle**, Version: a0.0.01
- **Internal/Memory/Fus_Handle.h**, Version: a0.0.01
