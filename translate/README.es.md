<div align="center">

# Fusion Code Engine

![Logo](../Documentation/img/FusionLogo.png)

**Motor de generación de código para arquitecturas CPU, en C.**

*Sin optimización implícita. Sin decisiones no solicitadas.*

[Quick Start](#quick-start) · [Filosofía](#filosofía) · [Conceptos](#conceptos) · [API](#referencia-de-la-api) · [Building](#building)

[English](README.en.md) · [Português](README.pt.md) · [Español](README.es.md) · [Русский](README.ru.md)

</div>

---

## Presentación

**Fusion** es un motor de generación de código para arquitecturas CPU, implementado en C.

El sistema recibe instrucciones en su propia representación — el **HIDR** — y las convierte en código máquina nativo mediante un conjunto de backends enchufables. El resultado puede ejecutarse directamente desde la memoria (**JIT**) o escribirse en disco (**AOT**).

```
   HIDR nodes          command chain         backend           linker          buffer
  ┌────────────┐      ┌──────────────────┐    ┌───────────┐    ┌────────────┐   ┌───────────┐
  │FusCodeMount│ ───▶ │ FusCommandRule   │ ─▶ │           │ ─▶ │ relocation │ ▶ │   bytes   │ ─▶ execute
  │  (the list)│      │      chain       │    │  encoder  │    │    patch   │   │ (JIT/AOT) │    the result
  └────────────┘      └──────────────────┘    └───────────┘    └────────────┘   └───────────┘
      the user           configuration          selection         registration        use
```

Fusion no decide cómo gestiona el usuario la memoria, cómo se analiza léxicamente su lenguaje, ni qué backend utilizar. Esas decisiones pertenecen a la aplicación: el motor ejecuta el pipeline exactamente como fue declarado.

## Filosofía

El proyecto parte de una posición deliberada frente a las herramientas convencionales de generación de código.

Sistemas como LLVM, los componentes internos de GCC y Cranelift se construyen con el objetivo de producir el mejor código posible. Operan optimizaciones, pases de transformación, análisis de flujo de datos y *inlining* agresivo. La premisa subyacente es que el programador no sabe expresar con precisión el resultado deseado, y que el compilador debe decidir por él.

**Fusion adopta el principio opuesto:**

> **El usuario sabe lo que desea generar. Fusion solo proporciona el medio para generarlo.**

En términos de comportamiento, esto significa:

- el HIDR no optimiza, reordena ni elimina las instrucciones proporcionadas;
- el backend ejecuta la codificación correspondiente a cada nodo, sin transformaciones;
- el linker aplica exclusivamente las relocaciones registradas por el backend, sin inferir referencias adicionales;
- cada etapa del pipeline ejecuta la operación solicitada y termina.

No existen pases de optimización, registros de flags ni parámetros de agresividad. Lo que no se solicita no se ejecuta.

La consecuencia es un sistema realmente **embebible**. «Emebrible» aquí no significa el sentido de LLVM, vincular decenas de bibliotecas y mantener cientos de megabytes de representación intermedia en memoria. Es embebible en el sentido directo: incluir los headers, llamar a las funciones, obtener los bytes, utilizarlos. No hay asignación implícita, comportamiento no documentado ni estado global expuesto.

El destinatario no es quien busca un compilador. Es **quien necesita generar código** — para emulación, para compilación JIT específica, para toolchains de hardware propietario, o para investigación. El contexto típico es el de alguien que se acercó a LLVM, evaluó la complejidad de integración y optó por una alternativa de superficie mínima.

En una formulación resumida:

> **Fusion está destinado a quien desea `GENERAR` código, no a quien delega en terceros la tarea de `OPTIMIZAR` código.**

### Comparación

| | LLVM / GCC | **Fusion** |
|---|---|---|
| Optimización de código | Silenciosa y agresiva | **Inexistente** |
| Reescritura de instrucciones | Sí | **No** |
| Decisiones implícitas | Numerosas | **Ninguna** — configuración explícita |
| Superficie de la API | Cientos de puntos de entrada | **~20 funciones `fus*`** |
| Curva de aprendizaje | Alta | **Un header, una cadena, una llamada** |
| Dependencias | Extensas | **libc** |
| Propio allocator | Difícil de reemplazar | **Una struct** |

La referencia de diseño es **Vulkan**: el usuario construye la cadena, el motor la respeta, y cada recurso posee una liberación explícita y correspondiente.

---

## Quick Start

Ejemplo completo y ejecutable: [`example/src/basic.c`](../example/src/basic.c)

```c
#include <Fusion/Fusion.h>

int main(void)
{
    // 1. Instancia (NULL = allocator del sistema)
    FusInstance  instance = NULL;
    FusCodeMount mount    = NULL;
    fusCreateInstance(&instance, NULL);
    fusCreateCodeMount(&instance, &mount);

    // 2. Instrucciones, en el orden deseado
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("BCL0"),              // dst: rbx
        FUS_HIDR_Sym("Print")));           // src: símbolo → relocación
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("OCL0"),              // rdi
        FUS_HIDR_Imm(30, HIDR_IMM64)));
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_CALL, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("BCL0"), FUS_HIDR_None()));
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_RET, HIDR_OP_SIZE_NONE,
        FUS_HIDR_None(), FUS_HIDR_None()));

    // 3. Backend
    FusModuleBackend backend = NULL;
    fusLoaderBackend(instance, &backend, "YOUR_BACKEND", FUS_BACKEND_TYPE_STATIC);

    // 4. Cadena de comandos (configuración del pipeline)
    FusCommandBackend cmd_backend = {
        .sType = FUS_COMMAND_SEND_BACKEND, .backend = backend, .pNext = NULL
    };
    FusCommandHidr cmd_hidr = {
        .sType = FUS_COMMAND_SEND_HIDR,   .code = mount,
        .pNext = (const FusCommandRuleBase_t*)&cmd_backend
    };

    // 5. Generación
    FusBackendReturn compiler = NULL;
    fusMountHidrsBytes(instance, (FusCommandRuleBase_t*)&cmd_hidr, &compiler);

    // 6. Resolución de símbolos (si se usan)
    FusLinkerContext linker = NULL;
    fusCreateLinkerContext(instance, &linker);
    fusAddSymbolLinker(linker, "Print", (uintptr_t)&Print);
    fusLinkerResolver((FusCommandRuleBase_t*)&cmd_backend, linker, compiler);

    // 7. Ejecución
    FusBufferContext_t* buffer = fusGetStreamBufferCompiler(instance, compiler);
    if (!fusExecutableBuffer(buffer)) {
        typedef void (*Fn)(void);
        ((Fn)buffer->buffer)();
    }

    // 8. Liberación — siempre, en orden inverso
    fusDestroyBufferCode(instance, buffer);
    fusDestroyBackendReturn(instance, compiler);
    fusDestroyBackend(instance, backend);
    fusDestroyLinkerContext(instance, linker);
    fusDestroyCodeMount(instance, mount);
    fusDestroyInstance(instance);
    return 0;
}
```

No hay registro de pases, flags de optimización ni rutinas de inicialización adicionales. Ocho pasos, el último siendo la liberación explícita de los recursos.

---

## Conceptos

### **HIDR** — la representación de instrucciones de Fusion

Un array de nodos. Cada nodo describe una instrucción mediante `opcode`, tamaño de operando y dos operandos (`dst` y `src`). No hay sistema de tipos, forma SSA ni etapa de verificación. El usuario declara las instrucciones y su orden.

```c
FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64, FUS_HIDR_Reg("BCL0"), FUS_HIDR_Imm(30, HIDR_IMM64))
```

> **Constructores de operando**: `FUS_HIDR_Reg`, `FUS_HIDR_Imm`, `FUS_HIDR_Mem`, `FUS_HIDR_Sym`, `FUS_HIDR_None`.
> **Opcodes disponibles**: `MOV`, `ADD`, `CMP`, `ADDR`, `CALL`, `PUSH`, `POP`, `RET`, `SYSCALL`.

### **Registros simbólicos** — identificación por función

El HIDR no expone identificadores de registros de la arquitectura. Los registros se designan por su función, y cada backend realiza la correspondencia con el hardware correspondiente.

```
   A C L 0
   │ │ │ └── índice
   │ │ └──── tamaño:  L = 64 bits
   │ └────── grupo:    C = propósito general
   └──────── papel:    A = acumulador
```

La composición de los campos, la tabla completa de caracteres y los ejemplos de mapeo están documentados en **[FusionRegistre](../Documentation/UserDocumentation/FusionRegistre.md)**.

El mismo HIDR puede enviarse a x86, ARM o RISC-V sin cambios, ya que cada backend traduce el papel declarado a su conjunto de registros.

### **Cadena de comandos** — la configuración del pipeline

El pipeline se configura mediante una lista enlazada al estilo `pNext` de Vulkan. Cada struct declara su papel mediante `sType` y apunta al siguiente elemento. El orden de los elementos es irrelevante, ya que Fusion recorre la cadena íntegramente.

| `sType` | Struct | Función |
|---|---|---|
| `FUS_COMMAND_SEND_BACKEND` | `FusCommandBackend` | Backend a utilizar |
| `FUS_COMMAND_SEND_HIDR` | `FusCommandHidr` | Mount de HIDR de entrada |
| `FUS_COMMAND_SEND_BUFFER` | `FusCommandBuffer` | Buffer de salida (reservado) |
| `FUS_COMMAND_SEND_LINKER` | — | Reservado |

---

## Referencia de la API

La API pública reside en `include/Fusion/`. El header único `Fusion/Fusion.h` agrega todas las declaraciones, y cada encabezado es pequeño y autocontenido:

| Header | Alcance |
|---|---|
| `Fusion/FusionTypes.h` | Tipos base, `FUS_API`, `FUS_DEFINE_HANDLE` |
| `Fusion/FusionInstance.h` | `fusCreateInstance` · `fusDestroyInstance` |
| `Fusion/FusionBuffer.h` | Creación, mapeo ejecutable, IO y liberación de buffers |
| `Fusion/FusionCompile.h` | `fusMountHidrsBytes` · `fusGetStreamBufferCompiler` |
| `Fusion/FusionRule.h` | Estructuras de la cadena de comandos |
| `Fusion/FusionTrace.h` | Árbol de rastreo de errores |
| `Fusion/Backend/FusionBackend.h` | Carga y liberación de backends |
| `Fusion/Linker/FusionLinkerInterface.h` | Símbolos, secciones y resolución de relocaciones |
| `Fusion/IRTypes/HidrType.h` | Nodos, operandos y opcodes del HIDR |
| `Fusion/IRTypes/HidrHelper.h` | `FusCodeMount` y constructores `FUS_HIDR_*` |
| `Fusion/IRTypes/HidrRegistre.h` | Registros simbólicos |
| `Fusion/IO/FusionGenericIO.h` · `FusionFileIO.h` | Sinks genéricos y de archivo |

Convenciones observadas en toda la API: las funciones reciben `FusInstance` como primer parámetro; los handles son opacos y typedefados mediante `FUS_DEFINE_HANDLE`; todo recurso creado posee una función `fusDestroy*` correspondiente; el estado de retorno es `FUSION_OK` (1) o `FUSION_ERRO` (0), y los detalles se consultan en el árbol de rastreo de la instancia.

---

## Backends

### Backend disponible

| Backend | Estado |
|---|---|
| [X86_Backend](https://github.com/Fusion-Toolchain/FusionBackendX86) | Activo — x86-64: `mov` `add` `cmp` `lea` `call` `push` `pop` `ret` `syscall` · relocaciones `REL32` y `ABS64` |

### Implementación de un Backend Estático

Un backend implementa dos funciones y se registra mediante una macro:

```c
#include <BackendInterface/Backend.h>

FusBackendInterface_t* MyBackendDefine(void)
{
    static FusBackendInterface_t interface = {
        .FUSI_BackendMountHidrArray   = my_mount_hidr,   // array HIDR -> bloque de datos
        .FUSI_BackendLinkerRelocation = my_relocate,    // aplica una relocación
    };
    return &interface;
}

REGISTER_BACKEND(My_Backend, MyBackendDefine);
```

El Core inyecta una `FusBackendApi_t`, que provee asignación, creación de bloques de salida, transferencias con destrucción explícita, registro de relocaciones y acceso al árbol de rastreo. Helpers disponibles: `FUSB_ALLOC` `FUSB_FREE` `FUSB_CREATE_BLOCK` `FUSB_CREATE_TRASNFER` `FUSB_REGISTRE_REALOCATION` `FUSB_GET_TRACE_FUSION`.

El tipo de relocación es **opaco** para el Core: su definición e interpretación pertenecen al backend. El Core no infiere ni modifica ese valor.

---

## Detalles técnicos

- **Allocator propio** — Fusion no invoca `malloc` directamente. Proporcione una `FusInstanceMyAllocation_t` (`Alloc` / `Free` / `Realloc` / `userdata`) o pase `NULL` para usar el allocator por defecto.
- **Árbol de rastreo** — cada paso interno registra un nodo en el árbol de errores de la instancia. `fusDumpTrace` muestra la cadena completa de causa y efecto, con archivo y línea.
- **Gestión de memoria** — subsistemas propios de Arena, Slab, Handle y LargerBlocks, operando sobre el allocator proporcionado.
- **Salida AOT** — `fusBufferIOSink` escribe los bytes en cualquier `FusIOSink`: ELF, binario plano o formato propio.

### Estructura de directorios

```
include/Fusion/          # API pública, subrepo
  Fusion.h               # header único — agrega todo
  FusionTypes.h          # tipos base, FUS_API, handles
  FusionInstance.h  FusionBuffer.h  FusionCompile.h  FusionRule.h  FusionTrace.h
  Backend/  Linker/  IRTypes/  IO/
include/BackendInterface/ Backend.h   # API para implementación de backends
include/Internal/                      # headers privados

src/Core/
  Compiler/              # HIDR -> backend -> buffer
  Linker_System/         # símbolos y resolución de relocaciones
  Memory/                # Arena, Slab, Handle, LargerBlocks
  Backend_System/        # registro y carga de backends
  BufferSystem/          # buffer de código y montaje ejecutable
  Error_Tree/  IO_Sytem/  IRTypes/
```

---

## Building

```bash
make -j 6              # genera libfusion.so
make example           # genera example_basic y example_program
make -C test test      # ejecuta los tests bajo valgrind
```

| Flag | Efecto |
|---|---|
| `DEBUG=Y` | Habilita rastreo `FUSION_DEBUG` en los allocators del core |

```bash
cd example
./example_basic     # JIT: función C invocada por código generado
./example_program   # AOT: produce output.elf
./output.elf        # → código de salida 1 (exit(60))
```
Requisitos: compilador con soporte C23 (gcc o clang) y Linux. Única dependencia: **libc**.

## Licencia

GPL-3.0 — consulte [LICENSE](../LICENSE).

---

<div align="center">
<sub>Fusion Code Engine — para quien desea <b>generar</b> código.</sub>
</div>
