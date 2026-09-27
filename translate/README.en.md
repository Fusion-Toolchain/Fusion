<div align="center">

# Fusion Code Engine

![Logo](../Documentation/img/FusionLogo.png)

**Code generation engine for CPU architectures, written in C.**

*No implicit optimization. No unrequested decisions.*

[Quick Start](#quick-start) · [Philosophy](#philosophy) · [Concepts](#concepts) · [API](#api-reference) · [Building](#building)

[English](translate/README.en.md) · [Português](translate/README.pt.md) · [Español](translate/README.es.md) · [Русский](translate/README.ru.md)

</div>

---

## Overview

**Fusion** is a code generation engine for CPU architectures, implemented in C.

The system receives instructions in its own representation — the **HIDR** — and converts them into native machine code through a set of pluggable backends. The result can be executed directly from memory (**JIT**) or written to disk (**AOT**).

```
   HIDR nodes          command chain         backend           linker          buffer
  ┌────────────┐      ┌──────────────────┐    ┌───────────┐    ┌────────────┐   ┌───────────┐
  │FusCodeMount│ ───▶ │ FusCommandRule   │ ─▶ │ X86 / ..  │ ─▶ │ relocation │ ▶ │   bytes   │ ─▶ execute
  │  (the list)│      │      chain       │    │  encoder  │    │    patch   │   │ (JIT/AOT) │    the result
  └────────────┘      └──────────────────┘    └───────────┘    └────────────┘   └───────────┘
      the user           configuration          selection         registration        use
```

Fusion does not decide how the user manages memory, how their language is lexed, nor which backend to use. Those decisions belong to the application: the engine runs the pipeline exactly as it was declared.

## Philosophy

The project starts from a deliberate position relative to conventional code generation tools.

Systems such as LLVM, the internal components of GCC and Cranelift are built with the goal of producing the best possible code. They run optimizations, transformation passes, data flow analysis and aggressive inlining. The premise underneath is that the programmer cannot precisely express the desired result, and that the compiler should decide for them.

**Fusion adopts the opposite principle:**

> **The user knows what they want to generate. Fusion only provides the means to generate it.**

In terms of behavior, this means:

- HIDR does not optimize, reorder or eliminate the instructions provided;
- the backend runs the encoding corresponding to each node, with no transformations;
- the linker applies exclusively the relocations registered by the backend, without inferring additional references;
- each pipeline step performs the requested operation and ends.

There are no optimization passes, no flag registers, no aggressiveness parameters. What is not requested is not executed.

The consequence is a genuinely **embeddable** system. "Embeddable" here does not mean the LLVM sense of vendoring dozens of libraries and keeping hundreds of megabytes of intermediate representation in memory. It is embeddable in the direct sense: include the headers, call the functions, obtain the bytes, use them. No implicit allocation, no undocumented behavior, no exposed global state.

The audience is not someone looking for a compiler. It is **someone who needs to generate code** — for emulation, for specific JIT compilation, for proprietary hardware toolchains, or for research. The typical context is that of someone who approached LLVM, evaluated the integration complexity, and chose an alternative with a minimal surface.

In a summarized formulation:

> **Fusion is meant for those who want to `GENERATE` code, not for those who delegate the task of `OPTIMIZING` code to a third party.**

### Comparison

| | LLVM / GCC | **Fusion** |
|---|---|---|
| Code optimization | Silent and aggressive | **Nonexistent** |
| Instruction rewriting | Yes | **No** |
| Implicit decisions | Numerous | **None** — explicit configuration |
| API surface | Hundreds of entry points | **~20 `fus*` functions** |
| Learning curve | High | **One header, one chain, one call** |
| Dependencies | Extensive | **libc** |
| Own allocator | Hard to replace | **One struct** |

The design reference is **Vulkan**: the user builds the chain, the engine respects it, and every resource has an explicit, matching release.

---

## Quick Start

Complete, runnable example: [`example/src/basic.c`](example/src/basic.c)

```c
#include <Fusion/Fusion.h>

int main(void)
{
    // 1. Instance (NULL = system allocator)
    FusInstance  instance = NULL;
    FusCodeMount mount    = NULL;
    fusCreateInstance(&instance, NULL);
    fusCreateCodeMount(&instance, &mount);

    // 2. Instructions, in the desired order
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("BCL0"),              // dst: rbx
        FUS_HIDR_Sym("Print")));           // src: symbol → relocation
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

    // 4. Command chain (pipeline configuration)
    FusCommandBackend cmd_backend = {
        .sType = FUS_COMMAND_SEND_BACKEND, .backend = x86, .pNext = NULL
    };
    FusCommandHidr cmd_hidr = {
        .sType = FUS_COMMAND_SEND_HIDR,   .code = mount,
        .pNext = (const FusCommandRuleBase_t*)&cmd_backend
    };

    // 5. Generation
    FusBackendReturn compiler = NULL;
    fusMountHidrsBytes(instance, (FusCommandRuleBase_t*)&cmd_hidr, &compiler);

    // 6. Symbol resolution (if used)
    FusLinkerContext linker = NULL;
    fusCreateLinkerContext(instance, &linker);
    fusAddSymbolLinker(linker, "Print", (uintptr_t)&Print);
    fusLinkerResolver((FusCommandRuleBase_t*)&cmd_backend, linker, compiler);

    // 7. Execution
    FusBufferContext_t* buffer = fusGetStreamBufferCompiler(instance, compiler);
    if (!fusExecutableBuffer(buffer)) {
        typedef void (*Fn)(void);
        ((Fn)buffer->buffer)();          // function calling generated code
    }

    // 8. Release — always, in reverse order
    fusDestroyBufferCode(instance, buffer);
    fusDestroyBackendReturn(instance, compiler);
    fusDestroyBackend(instance, x86);
    fusDestroyLinkerContext(instance, linker);
    fusDestroyCodeMount(instance, mount);
    fusDestroyInstance(instance);
    return 0;
}
```

No pass registry, no optimization flags, no additional initialization routines. Eight steps, the last one being the explicit release of resources.

Other examples:

| Example | Purpose |
|---|---|
| [`example/src/basic.c`](example/src/basic.c) | JIT with linker: generated code calls a C function |
| [`example/src/program.c`](example/src/program.c) | AOT: produces a runnable ELF file |

---

## Concepts

### **HIDR** — Fusion's instruction representation

An array of nodes. Each node describes one instruction through `opcode`, operand size and two operands (`dst` and `src`). There is no type system, no SSA form, no verification step. The user declares the instructions and their order.

```c
FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64, FUS_HIDR_Reg("BCL0"), FUS_HIDR_Imm(30, HIDR_IMM64))
```

Operand constructors: `FUS_HIDR_Reg` · `FUS_HIDR_Imm` · `FUS_HIDR_Mem` · `FUS_HIDR_Sym` · `FUS_HIDR_None`

Available opcodes: `MOV` `ADD` `CMP` `ADDR` (lea) `CALL` `PUSH` `POP` `RET` `SYSCALL`

### **Symbolic registers** — identification by role

HIDR does not expose architecture register identifiers. Registers are designated by their role, and each backend performs the mapping to the corresponding hardware.

```
   A C L 0
   │ │ │ └── index
   │ │ └──── size:  L = 64 bits
   │ └────── group:  C = general purpose
   └──────── role:   A = accumulator
```

Field composition, the complete character table and mapping examples are documented in **[Documentation/UserDocumentation/FusionRegistre.md](Documentation/UserDocumentation/FusionRegistre.md)**.

The same HIDR can be submitted to x86, ARM or RISC-V without change, since each backend translates the declared role into its own register set.

### **Command chain** — pipeline configuration

The pipeline is configured by a linked list in the Vulkan `pNext` style. Each struct declares its role through `sType` and points to the next element. The order of the elements is irrelevant, since Fusion walks the whole chain.

| `sType` | Struct | Function |
|---|---|---|
| `FUS_COMMAND_SEND_BACKEND` | `FusCommandBackend` | Backend to be used |
| `FUS_COMMAND_SEND_HIDR` | `FusCommandHidr` | Input HIDR mount |
| `FUS_COMMAND_SEND_BUFFER` | `FusCommandBuffer` | Output buffer (reserved) |
| `FUS_COMMAND_SEND_LINKER` | — | Reserved |

---

## API Reference

The public API lives in `include/Fusion/`. The single header `Fusion/Fusion.h` aggregates every declaration, and each header is small and self-contained:

| Header | Scope |
|---|---|
| `Fusion/FusionTypes.h` | Base types, `FUS_API`, `FUS_DEFINE_HANDLE` |
| `Fusion/FusionInstance.h` | `fusCreateInstance` · `fusDestroyInstance` |
| `Fusion/FusionBuffer.h` | Buffer creation, executable mapping, IO and release |
| `Fusion/FusionCompile.h` | `fusMountHidrsBytes` · `fusGetStreamBufferCompiler` |
| `Fusion/FusionRule.h` | Command chain structures |
| `Fusion/FusionTrace.h` | Error trace tree |
| `Fusion/Backend/FusionBackend.h` | Backend loading and release |
| `Fusion/Linker/FusionLinkerInterface.h` | Symbols, sections and relocation resolution |
| `Fusion/IRTypes/HidrType.h` | HIDR nodes, operands and opcodes |
| `Fusion/IRTypes/HidrHelper.h` | `FusCodeMount` and the `FUS_HIDR_*` constructors |
| `Fusion/IRTypes/HidrRegistre.h` | Symbolic registers |
| `Fusion/IO/FusionGenericIO.h` · `FusionFileIO.h` | Generic and file sinks |

Conventions observed across the whole API: functions take `FusInstance` as their first parameter; handles are opaque and typedef'd through `FUS_DEFINE_HANDLE`; every created resource has a matching `fusDestroy*` function; the return status is `FUSION_OK` (1) or `FUSION_ERRO` (0), and details are read from the instance trace tree.

---

## Backends

### Available backend

| Backend | Status |
|---|---|
| `X86_Backend` | Active — x86-64: `mov` `add` `cmp` `lea` `call` `push` `pop` `ret` `syscall` · `REL32` and `ABS64` relocations |

### Implementing a Static Backend

A backend implements two functions and registers itself through a macro:

```c
#include <BackendInterface/Backend.h>

FusBackendInterface_t* MyBackendDefine(void)
{
    static FusBackendInterface_t interface = {
        .FUSI_BackendMountHidrArray   = my_mount_hidr,   // HIDR array -> data block
        .FUSI_BackendLinkerRelocation = my_relocate,    // applies one relocation
    };
    return &interface;
}

REGISTER_BACKEND(My_Backend, MyBackendDefine);
```

The Core injects a `FusBackendApi_t`, which provides allocation, output block creation, transfers with explicit destruction, relocation registration and access to the trace tree. Available helpers: `FUSB_ALLOC` `FUSB_FREE` `FUSB_CREATE_BLOCK` `FUSB_CREATE_TRASNFER` `FUSB_REGISTRE_REALOCATION` `FUSB_GET_TRACE_FUSION`.

The relocation type is **opaque** to the Core: its definition and interpretation belong to the backend. The Core neither infers nor modifies that value.

---

## Technical Details

- **Own allocator** — Fusion does not call `malloc` directly. Provide a `FusInstanceMyAllocation_t` (`Alloc` / `Free` / `Realloc` / `userdata`) or pass `NULL` to use the default allocator.
- **Trace tree** — every internal step registers a node in the instance error tree. `fusDumpTrace` shows the full cause and effect chain, with file and line.
- **Memory management** — own Arena, Slab, Handle and LargerBlocks subsystems, operating on the supplied allocator.
- **AOT output** — `fusBufferIOSink` writes the bytes to any `FusIOSink`: ELF, flat binary or a custom format.

### Directory structure

```
include/Fusion/          # public API
  Fusion.h               # single header — aggregates everything
  FusionTypes.h          # base types, FUS_API, handles
  FusionInstance.h  FusionBuffer.h  FusionCompile.h  FusionRule.h  FusionTrace.h
  Backend/  Linker/  IRTypes/  IO/
include/BackendInterface/ Backend.h   # API for backend implementations
include/Internal/                      # private headers

src/Backend/X86/          # encoder and instruction sets
src/Core/
  Compiler/              # HIDR -> backend -> buffer
  Linker_System/         # symbols and relocation resolution
  Memory/                # Arena, Slab, Handle, LargerBlocks
  Backend_System/        # backend registration and loading
  BufferSystem/          # code buffer and executable mounting
  Error_Tree/  IO_Sytem/  IRTypes/
```

---

## Building

```bash
make -j 6              # builds libfusion.so
make example           # builds example_basic and example_program
make -C test test      # runs the tests under valgrind
```

| Flag | Effect |
|---|---|
| `DEBUG=Y` | Enables `FUSION_DEBUG` tracing in the core allocators |

```bash
cd example
./example_basic     # JIT: C function invoked by generated code
./example_program   # AOT: produces output.elf
./output.elf        # → exit code 1 (exit(60))
```

Requirements: a C23 capable compiler (gcc or clang) and Linux. Only dependency: **libc**.

---

## License

GPL-3.0 — see [LICENSE](LICENSE).

---

<div align="center">
<sub>Fusion Code Engine — for those who want to <b>generate</b> code.</sub>
</div>
