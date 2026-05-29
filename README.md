# Fusion

![Logo](Documentation/img/FusionLogo.png)

<p align="center">
  <b>A low-level compiler backend and code generation toolchain for CPU (and GPU in the future).</b><br/>
  Custom IR · Custom Linker · Pluggable Backends · JIT & AOT
</p>

---

## Overview

Fusion is a modular code generation engine written in C. It takes an intermediate representation called **HIDR** and compiles it down to native machine code through a pluggable backend system. The design philosophy is inspired by Vulkan: explicit control, no hidden magic, the caller owns the pipeline.

```
HIDR Nodes  →  FusCommandRule chain  →  FUS_MountHidrsBytes  →  FusBufferContext_t  →  Execute
```

Fusion does **not** dictate how you manage memory, how you parse your language, or which backend you use. You wire it together.

---

## Features

- **Custom x86 backend** — native code generation for x86, with `mov`, `add`, `call`, `lea`, `ret` and growing instruction coverage
- **Pluggable backend system** — load backends by name at runtime (`FUS_BACKEND_TYPE_STATIC`, dynamic planned)
- **Custom linker** — symbol resolution via internal hashtable pipeline
- **Custom allocator support** — bring your own `Alloc`/`Free`/`Realloc` via `FusInstanceMyAllocation_t`
- **JIT execution** — compile to an executable buffer and call it directly
- **HIDR IR** — compact node-array intermediate representation feeding the backend

---

## Project Structure

```
include/Fusion/          # Public API headers
  Fusion.h               # Main entry point
  FusionTypes.h          # Core types (FusInstance, FusBufferContext_t, FusStatusFlag_t)
  FusionRule.h           # Command rule chain (FusCommandRuleBase_t and friends)
  Backend/FusionBackend.h
  Linker/FusionLinkerInterface.h
  IRTypes/HidrType.h

src/
  Backend/X86/           # x86 instruction emitters and pipeline
  Core/
    Compiler/            # compiler_pipeline.c — HIDR → backend → buffer
    Linker_System/       # Symbol resolution
    Memory/              # Arena, Slab, Handle, LargerBlocks
    Backend_System/      # Backend loader and injector
```

---

## Quick Start

### 1. Create an instance

```c
#include <Fusion/Fusion.h>

// Use the default system allocator
FusInstanceMyAllocation_t alloc = {
    .Alloc   = my_alloc,
    .Free    = my_free,
    .Realloc = my_realloc,
    .userdata = NULL,
};

FusInstance instance;
FusStatusFlag_t status = FUS_CreateInstance(&instance, &alloc);
if (status != FUSION_OK) {
    fprintf(stderr, "FUS_CreateInstance: %s\n", FUS_StrError(status));
    return 1;
}
```

### 2. Load a backend

```c
FusModuleBackend_t backend;
status = FUS_LoaderBackend(&instance, &backend, "x86", FUS_BACKEND_TYPE_STATIC);
if (status != FUSION_OK) { /* handle */ }
```

### 3. Build the command rule chain

The compiler pipeline is driven by a `pNext` chain of `FusCommandRuleBase_t` structs — similar to Vulkan's `pNext` extension chain.

```c
// Provide the output buffer
FusBufferContext_t* out_buf = FUS_CreateBufferCode(4096);

FusCommandBuffer cmd_buf = {
    .sType  = FUS_COMMAND_SEND_BUFFER,
    .pNext  = NULL,
    .buffer = out_buf,
};

// Provide your HIDR node array
FusCommandHidr cmd_hidr = {
    .sType      = FUS_COMMAND_SEND_HIDR,
    .pNext      = (FusCommandRuleBase_t*)&cmd_buf,
    .hidr_arry  = my_hidr_nodes,
    .hidr_count = my_node_count,
};

// Provide the backend
FusCommandBackend cmd_backend = {
    .sType   = FUS_COMMAND_SEND_BACKEND,
    .pNext   = (FusCommandRuleBase_t*)&cmd_hidr,
    .backend = backend,
};
```

### 4. Compile

```c
FusBackendReturn_t* result = FUS_MountHidrsBytes(&instance, (FusCommandRuleBase_t*)&cmd_backend);

FusBufferContext_t* exec_buf = FUS_GetStreamBufferCompiler(result);
```

### 5. Execute (JIT)

```c
FUS_ExecutableBuffer(exec_buf);   // marks buffer as executable (mprotect)

typedef int (*JitFn)(void);
JitFn fn = (JitFn)exec_buf->buffer;
int ret = fn();
```

### 6. Cleanup

```c
FUS_DestroyCompiler(instance, result);
FUS_DestroyBufferCode(exec_buf);
FUS_DestroyBackend(backend);
FUS_DestroyInstance(&instance);
```

---

## API Reference

### Instance

| Function | Description |
|---|---|
| `FUS_CreateInstance(ctx, allocation)` | Create a Fusion instance with a custom allocator |
| `FUS_DestroyInstance(ctx)` | Destroy instance and free all associated resources |

### Buffer

| Function | Description |
|---|---|
| `FUS_CreateBufferCode(size)` | Allocate a code buffer of `size` bytes |
| `FUS_ExecutableBuffer(buffer)` | Mark buffer as executable (call before JIT execution) |
| `FUS_DestroyBufferCode(buffer)` | Free a code buffer |

### Backend

| Function | Description |
|---|---|
| `FUS_LoaderBackend(instance, ctx, name, type)` | Load a backend module by name |
| `FUS_DestroyBackend(backend)` | Unload and free a backend module |

### Compilation

| Function | Description |
|---|---|
| `FUS_MountHidrsBytes(instance, rule)` | Run the compiler pipeline from a command rule chain |
| `FUS_GetStreamBufferCompiler(ctx_backend)` | Extract the output buffer from a compilation result |
| `FUS_DestroyCompiler(instance, ctx_backend)` | Free compilation result resources |

### Utilities

| Function | Description |
|---|---|
| `FUS_StrError(status)` | Convert a `FusStatusFlag_t` to a human-readable string |

---

## Command Rule Chain

The compilation pipeline is configured via a linked chain of command structs. Each struct has an `sType` identifying its role and a `pNext` pointing to the next rule in the chain.

| Type | Struct | Purpose |
|---|---|---|
| `FUS_COMMAND_SEND_BUFFER` | `FusCommandBuffer` | Output buffer for generated code |
| `FUS_COMMAND_SEND_HIDR` | `FusCommandHidr` | Input HIDR node array |
| `FUS_COMMAND_SEND_BACKEND` | `FusCommandBackend` | Backend module to use |
| `FUS_COMMAND_SEND_LINKER` | *(planned)* | Linker configuration |

---

## Status Codes

```c
FUSION_OK   = 0   // Success
FUSION_ERRO = 1   // Generic error — use FUS_StrError() for details
```

---

## Custom Allocator

Fusion never calls `malloc`/`free` directly. You provide the allocator at instance creation:

```c
typedef struct {
    void* (*Alloc)  (void* userdata, size_t size);
    void  (*Free)   (void* userdata, void* ptr);
    void* (*Realloc)(void* userdata, void* old_ptr, size_t old_size, size_t new_size);
    void* userdata;
} FusInstanceMyAllocation_t;
```

---

## Backends

| Backend | Status |
|---|---|
| x86 | ✅ Active — `mov`, `add`, `call`, `lea`, `ret` |
| GPU / SPIR-V / PTX | 🔮 Planned |

---

## Building

```bash
make
```

The Makefile targets the project as a static library. See `src/linker.ld` for the custom linker script.

---

## License

GPL-3.0 — see [LICENSE](LICENSE).