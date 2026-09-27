<div align="center">

# Fusion Code Engine

![Logo](Documentation/img/FusionLogo.png)

**Motor de geração de código para CPU, em C.**

*Sem otimização implícita. Sem decisões não solicitadas.*

[Quick Start](#quick-start) · [Filosofia](#filosofia) · [Conceitos](#conceitos) · [API](#referência-da-api) · [Building](#building)

</div>

---

## Apresentação

**Fusion** é um motor de geração de código para arquitetura CPU, implementado em C.

O sistema recebe instruções em sua representação própria — o **HIDR** — e as converte em código de máquina nativo por meio de um conjunto de backends plugáveis. O resultado pode ser executado diretamente na memória (**JIT**) ou gravado em disco (**AOT**).

```
   HIDR nodes          cadeia de comandos       backend            linker          buffer
  ┌────────────┐      ┌──────────────────┐    ┌───────────┐    ┌────────────┐   ┌───────────┐
  │FusCodeMount│ ───▶ │ FusCommandRule   │ ─▶ │ X86 / ..  │ ─▶ │  relocação │ ▶ │   bytes   │ ─▶ executar
  │  (a lista) │      │      chain       │    │  encoder  │    │   patch    │   │ (JIT/AOT)│
  └────────────┘      └──────────────────┘    └───────────┘    └────────────┘   └───────────┘
      o usuário             configuração             seleção             registro          uso
```

O Fusion não determina como o usuário gerencia memória, como realiza a análise léxica de sua linguagem, nem qual backend empregar. Essas decisões pertencem à aplicação: o motor executa a pipeline exatamente como ela foi declarada.

---

## Filosofia

O projeto parte de um posicionamento deliberado em relação às ferramentas de geração de código convencionais.

Sistemas como LLVM, os componentes internos do GCC e Cranelift são construídos com o objetivo de produzir o melhor código possível. Operam otimizações, passes de transformação, análise de fluxo de dados e inlining agressivo. A premissa subjacente é que o programador não sabe expressar com precisão o resultado desejado, e que o compilador deve decidir por ele.

**Fusion adota o princípio oposto:**

> **O usuário sabe o que deseja gerar. O Fusion apenas fornece o meio para gerá-lo.**

Em termos de comportamento, isso significa que:

- o HIDR não otimiza, reordena ou elimina as instruções fornecidas;
- o backend executa a codificação correspondente a cada nó, sem transformações;
- o linker aplica exclusivamente as relocações registradas pelo backend, sem inferir referências adicionais;
- cada etapa do pipeline executa a operação solicitada e encerra.

Não existem passes de otimização, registros de flags ou parâmetros de agressividade. O que não é solicitado não é executado.

A consequência é um sistema efetivamente **embutível**. A acepção de "embutível" aqui não é a do LLVM, que implica vincular dezenas de bibliotecas e manter centenas de megabytes de representação intermediária em memória. É embutível no sentido direto: incluir os headers, chamar as funções, obter os bytes, utilizá-los. Não há alocação implícita, comportamento não documentado nem estado global exposto.

O destinatário não é quem busca um compilador. É **quem precisa gerar código** — para emulação, para compilação JIT específica, para toolchains de hardware proprietário, ou para pesquisa. O contexto de uso típico é o de quem se aproximou do LLVM, avaliou a complexidade de integração e optou por uma alternativa de superfície mínima.

Em formulação resumida:

> **Fusion destina-se a quem deseja `GERAR` código, e não a quem delega a terceiros a tarefa de `OTIMIZAR` código.**

### Comparação

| | LLVM / GCC | **Fusion** |
|---|---|---|
| Otimização de código | Silenciosa e agressiva | **Inexistente** |
| Reescrita de instruções | Sim | **Não** |
| Decisões implícitas | Numerosas | **Nenhuma** — configuração explícita |
| Superfície da API | Centenas de entradas | **~20 funções `fus*`** |
| Curva de aprendizado | Alta | **Um header, uma cadeia, uma chamada** |
| Dependências | Extensas | **libc** |
| Alocador próprio | Difícil de substituir | **Uma struct** |

A referência de design é o **Vulkan**: o usuário constrói a cadeia, o motor a respeita, e cada recurso possui um descarte explícito e correspondente.

---

## Quick Start

Exemplo completo e executável: [`example/src/basic.c`](example/src/basic.c)

```c
#include <Fusion/Fusion.h>

int main(void)
{
    // 1. Instância (NULL = alocador do sistema)
    FusInstance  instance = NULL;
    FusCodeMount mount    = NULL;
    fusCreateInstance(&instance, NULL);
    fusCreateCodeMount(&instance, &mount);

    // 2. Instruções, na ordem desejada
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("BCL0"),              // dst: rbx
        FUS_HIDR_Sym("Print")));           // src: símbolo → relocação
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("OCL0"),              // rdi
        FUS_HIDR_Imm(30, HIDR_IMM64)));
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_CALL, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("BCL0"), FUS_HIDR_None()));
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_RET, HIDR_OP_SIZE_NONE,
        FUS_HIDR_None(), FUS_HIDR_None()));

    // 3. Backend
    FusModuleBackend x86 = NULL;
    fusLoaderBackend(instance, &x86, "X86_Backend", FUS_BACKEND_TYPE_STATIC);

    // 4. Cadeia de comandos (configuração da pipeline)
    FusCommandBackend cmd_backend = {
        .sType = FUS_COMMAND_SEND_BACKEND, .backend = x86, .pNext = NULL
    };
    FusCommandHidr cmd_hidr = {
        .sType = FUS_COMMAND_SEND_HIDR,   .code = mount,
        .pNext = (const FusCommandRuleBase_t*)&cmd_backend
    };

    // 5. Geração
    FusBackendReturn compiler = NULL;
    fusMountHidrsBytes(instance, (FusCommandRuleBase_t*)&cmd_hidr, &compiler);

    // 6. Resolução de símbolos (se utilizados)
    FusLinkerContext linker = NULL;
    fusCreateLinkerContext(instance, &linker);
    fusAddSymbolLinker(linker, "Print", (uintptr_t)&Print);
    fusLinkerResolver((FusCommandRuleBase_t*)&cmd_backend, linker, compiler);

    // 7. Execução
    FusBufferContext_t* buffer = fusGetStreamBufferCompiler(instance, compiler);
    if (!fusExecutableBuffer(buffer)) {
        typedef void (*Fn)(void);
        ((Fn)buffer->buffer)();          // função chamando código gerado
    }

    // 8. Descarte — sempre, na ordem inversa
    fusDestroyBufferCode(instance, buffer);
    fusDestroyBackendReturn(instance, compiler);
    fusDestroyBackend(instance, x86);
    fusDestroyLinkerContext(instance, linker);
    fusDestroyCodeMount(instance, mount);
    fusDestroyInstance(instance);
    return 0;
}
```

Não há registro de passes, flags de otimização ou rotinas de inicialização suplementares. Oito etapas, das quais a última é o descarte explícito dos recursos.

Outros exemplos:

| Exemplo | Objetivo |
|---|---|
| [`example/src/basic.c`](example/src/basic.c) | JIT com linker: código gerado invoca função C |
| [`example/src/program.c`](example/src/program.c) | AOT: produção de um arquivo ELF executável |

---

## Conceitos

### **HIDR** — a representação de instruções do Fusion

Um array de nós. Cada nó descreve uma instrução por meio de `opcode`, tamanho de operando e dois operandos (`dst` e `src`). Não há sistema de tipos, forma SSA ou etapa de verificação. O usuário declara as instruções e sua ordem.

```c
FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64, FUS_HIDR_Reg("BCL0"), FUS_HIDR_Imm(30, HIDR_IMM64))
```

Construtores de operando: `FUS_HIDR_Reg` · `FUS_HIDR_Imm` · `FUS_HIDR_Mem` · `FUS_HIDR_Sym` · `FUS_HIDR_None`

Opcodes disponíveis: `MOV` `ADD` `CMP` `ADDR` (lea) `CALL` `PUSH` `POP` `RET` `SYSCALL`

### **Registradores simbólicos** — identificação por função

O HIDR não expõe identificadores de registradores da arquitetura. Registradores são designados por sua função, e cada backend realiza a correspondência para o hardware correspondente.

```
   A C L 0
   │ │ │ └── índice
   │ │ └──── tamanho:  L = 64 bits
   │ └────── grupo:    C = general purpose
   └──────── papel:    A = acumulador
```

A composição dos campos, a tabela completa de caracteres e os exemplos de mapeamento estão documentados em **[Documentation/UserDocumentation/FusionRegistre.md](Documentation/UserDocumentation/FusionRegistre.md)**.

O mesmo HIDR pode ser submetido a x86, ARM ou RISC-V sem alteração, uma vez que cada backend traduz o papel declarado para o seu conjunto de registradores.

### **Cadeia de comandos** — a configuração da pipeline

A pipeline é configurada por uma lista encadeada no padrão `pNext` do Vulkan. Cada struct declara seu papel por meio de `sType` e aponta para o próximo elemento. A ordem dos elementos é irrelevante, pois o Fusion percorre a cadeia integralmente.

| `sType` | Struct | Função |
|---|---|---|
| `FUS_COMMAND_SEND_BACKEND` | `FusCommandBackend` | Backend a ser utilizado |
| `FUS_COMMAND_SEND_HIDR` | `FusCommandHidr` | Mount de HIDR de entrada |
| `FUS_COMMAND_SEND_BUFFER` | `FusCommandBuffer` | Buffer de saída (reservado) |
| `FUS_COMMAND_SEND_LINKER` | — | Reservado |

---

## Referência da API

A API pública reside em `include/Fusion/`. O header único `Fusion/Fusion.h` agrega todas as declarações, e cada cabeçalho é pequeno e autocontido:

| Header | Escopo |
|---|---|
| `Fusion/FusionTypes.h` | Tipos base, `FUS_API`, `FUS_DEFINE_HANDLE` |
| `Fusion/FusionInstance.h` | `fusCreateInstance` · `fusDestroyInstance` |
| `Fusion/FusionBuffer.h` | Criação, mapeamento executável, IO e descarte de buffers |
| `Fusion/FusionCompile.h` | `fusMountHidrsBytes` · `fusGetStreamBufferCompiler` |
| `Fusion/FusionRule.h` | Estruturas da cadeia de comandos |
| `Fusion/FusionTrace.h` | Árvore de rastreamento de erros |
| `Fusion/Backend/FusionBackend.h` | Carga e descarte de backends |
| `Fusion/Linker/FusionLinkerInterface.h` | Símbolos, seções e resolução de relocações |
| `Fusion/IRTypes/HidrType.h` | Nós, operandos e opcodes do HIDR |
| `Fusion/IRTypes/HidrHelper.h` | `FusCodeMount` e construtores `FUS_HIDR_*` |
| `Fusion/IRTypes/HidrRegistre.h` | Registradores simbólicos |
| `Fusion/IO/FusionGenericIO.h` · `FusionFileIO.h` | Sinks genéricos e de arquivo |

Convenções observadas em toda a API: as funções recebem `FusInstance` como primeiro parâmetro; handles são opacos e typedefados por `FUS_DEFINE_HANDLE`; todo recurso criado possui uma função `fusDestroy*` correspondente; o status de retorno é `FUSION_OK` (1) ou `FUSION_ERRO` (0), e os detalhes são consultados na árvore de rastreamento da instância.

---

## Backends

### Backend disponível

| Backend | Estado |
|---|---|
| `X86_Backend` | Ativo — x86-64: `mov` `add` `cmp` `lea` `call` `push` `pop` `ret` `syscall` · relocações `REL32` e `ABS64` |

### Implementação de um Backend Estatico

Um backend implementa duas funções e registra-se por meio de uma macro:

```c
#include <BackendInterface/Backend.h>

FusBackendInterface_t* MyBackendDefine(void)
{
    static FusBackendInterface_t interface = {
        .FUSI_BackendMountHidrArray   = my_mount_hidr,   // array HIDR -> bloco de dados
        .FUSI_BackendLinkerRelocation = my_relocate,    // aplica uma relocação
    };
    return &interface;
}

REGISTER_BACKEND(My_Backend, MyBackendDefine);
```

O Core injeta uma `FusBackendApi_t`, que provê alocação, criação de blocos de saída, transfers com destrutor explícito, registro de relocações e acesso à árvore de rastreamento. Helpers disponíveis: `FUSB_ALLOC` `FUSB_FREE` `FUSB_CREATE_BLOCK` `FUSB_CREATE_TRASNFER` `FUSB_REGISTRE_REALOCATION` `FUSB_GET_TRACE_FUSION`.

O tipo de relocação é **opaco** para o Core: sua definição e interpretação pertencem ao backend. O Core não infere nem modifica esse valor.

---

## Detalhes técnicos

- **Alocador próprio** — o Fusion não invoca `malloc` diretamente. Forneça uma `FusInstanceMyAllocation_t` (`Alloc` / `Free` / `Realloc` / `userdata`) ou passe `NULL` para utilizar o alocador padrão.
- **Árvore de rastreamento** — cada etapa interna registra um nó na árvore de erros da instância. `fusDumpTrace` apresenta a cadeia completa de causa e efeito, com arquivo e linha.
- **Gerência de memória** — subsistemas próprios de Arena, Slab, Handle e LargerBlocks, operando sobre o alocador fornecido.
- **Saída AOT** — `fusBufferIOSink` grava os bytes em qualquer `FusIOSink`: ELF, binário plano ou formato próprio.

### Estrutura de diretórios

```
include/Fusion/          # API pública
  Fusion.h               # header único — agrega todos
  FusionTypes.h          # tipos base, FUS_API, handles
  FusionInstance.h  FusionBuffer.h  FusionCompile.h  FusionRule.h  FusionTrace.h
  Backend/  Linker/  IRTypes/  IO/
include/BackendInterface/ Backend.h   # API para implementação de backends
include/Internal/                      # headers privados

src/Backend/X86/          # encoder e conjuntos de instruções
src/Core/
  Compiler/              # HIDR -> backend -> buffer
  Linker_System/         # símbolos e resolução de relocações
  Memory/                # Arena, Slab, Handle, LargerBlocks
  Backend_System/        # registro e carga de backends
  BufferSystem/          # buffer de código e montagem executável
  Error_Tree/  IO_Sytem/  IRTypes/
```

---

## Building

```bash
make -j 6              # gera libfusion.so
make example           # gera example_basic e example_program
make -C test test      # executa os testes sob valgrind
```

| Flag | Efeito |
|---|---|
| `DEBUG=Y` | Habilita rastreamento `FUSION_DEBUG` nos alocadores do core |

```bash
cd example
./example_basic     # JIT: função C invocada por código gerado
./example_program   # AOT: produz output.elf
./output.elf        # → código de saída 1 (exit(60))
```

Requisitos: compilador com suporte a C23 (gcc ou clang) e Linux. Única dependência: **libc**.

---

## License

GPL-3.0 — consulte [LICENSE](LICENSE).

---

<div align="center">
<sub>Fusion Code Engine — para quem deseja <b>gerar</b> código.</sub>
</div>
