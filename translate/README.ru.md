<div align="center">

# Fusion Code Engine

![Logo](../Documentation/img/FusionLogo.png)

**Движок генерации машинного кода для CPU-архитектур, написанный на C.**

*Без неявной оптимизации. Без незапрошенных решений.*

[Quick Start](#quick-start) · [Философия](#философия) · [Концепции](#концепции) · [API](#справочник-api) · [Building](#building)

[English](README.en.md) · [Português](README.pt.md) · [Español](README.es.md) · [Русский](README.ru.md)

</div>

---

## Описание

**Fusion** — это движок генерации машинного кода для CPU-архитектур, реализованный на C.

Система принимает инструкции в собственном представлении — **HIDR** — и преобразует их в нативный машинный код через набор подключаемых бэкендов. Результат может исполняться прямо из памяти (**JIT**) или записываться на диск (**AOT**).

```
   HIDR nodes          command chain         backend           linker          buffer
  ┌────────────┐      ┌──────────────────┐    ┌───────────┐    ┌────────────┐   ┌───────────┐
  │FusCodeMount│ ───▶ │ FusCommandRule   │ ─▶ │           │ ─▶ │ relocation │ ▶ │   bytes   │ ─▶ execute
  │  (the list)│      │      chain       │    │  encoder  │    │    patch   │   │ (JIT/AOT) │    the result
  └────────────┘      └──────────────────┘    └───────────┘    └────────────┘   └───────────┘
      the user           configuration          selection         registration        use
```

Fusion не решает, как пользователь управляет памятью, как выполняется лексический разбор его языка и какой бэкенд использовать. Это решения принадлежат приложению: движок выполняет конвейер в точности так, как тот был объявлен.

## Философия

Проект исходит из осознанной позиции по отношению к обычным инструментам генерации кода.

Системы вроде LLVM, внутренних компонентов GCC и Cranelift строятся с целью получить наилучший возможный код. Они выполняют оптимизации, трансформационные проходы, анализ потоков данных и агрессивный инлайнинг. В основе этого лежит предположение, что программист не умеет точно выразить желаемый результат и что компилятор должен решить за него.

**Fusion принимает противоположный принцип:**

> **Пользователь знает, что хочет сгенерировать. Fusion лишь предоставляет средство для этого.**

В поведенческом смысле это означает:

- HIDR не оптимизирует, не переупорядочивает и не удаляет переданные инструкции;
- бэкенд выполняет кодирование, соответствующее каждому узлу, без преобразований;
- линкер применяет исключительно те релокации, которые зарегистрировал бэкенд, не выводя дополнительных ссылок;
- каждый шаг конвейера выполняет запрошенную операцию и завершается.

В проекте нет проходов оптимизации, регистров флагов и параметров агрессивности. Что не запрошено — не выполняется.

Следствие — действительно **встраиваемая** система. Здесь «встраиваемость» не означает смысл из LLVM, где связываются десятки библиотек и в памяти удерживаются сотни мегабайт промежуточного представления. Она встраиваема в прямом смысле: подключите заголовки, вызовите функции, получите байты, используйте их. Никаких неявных выделений памяти, никакого недокументированного поведения, никакого доступного глобального состояния.

Адресат — не тот, кто ищет компилятор. Это **тот, кому нужно генерировать код** — для эмуляции, для специализированной JIT-компиляции, для проприетарных аппаратных тулчейнов или для исследований. Типичный контекст: человек, приблизившийся к LLVM, оценивший сложность интеграции и выбравший альтернативу с минимальной поверхностью.

В краткой формулировке:

> **Fusion предназначен для того, кто хочет `ГЕНЕРИРОВАТЬ` код, а не для того, кто делегирует задачу `ОПТИМИЗАЦИИ` кода третьим лицам.**

### Сравнение

| | LLVM / GCC | **Fusion** |
|---|---|---|
| Оптимизация кода | Тихая и агрессивная | **Отсутствует** |
| Переписывание инструкций | Да | **Нет** |
| Неявные решения | Многочисленны | **Нет** — явная конфигурация |
| Поверхность API | Сотни точек входа | **~20 функций `fus*`** |
| Кривая обучения | Высокая | **Один заголовок, одна цепочка, один вызов** |
| Зависимости | Обширные | **libc** |
| Собственный аллокатор | Сложно заменить | **Одна структура** |

Проектным ориентиром служит **Vulkan**: пользователь строит цепочку, движок её уважает, а каждый ресурс имеет явное и соответствующее освобождение.

---

## Quick Start

Полный рабочий пример: [`example/src/basic.c`](../example/src/basic.c)

```c
#include <Fusion/Fusion.h>

int main(void)
{
    // 1. Инстанс (NULL = системный аллокатор)
    FusInstance  instance = NULL;
    FusCodeMount mount    = NULL;
    fusCreateInstance(&instance, NULL);
    fusCreateCodeMount(&instance, &mount);

    // 2. Инструкции в желаемом порядке
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("BCL0"),              // dst: rbx
        FUS_HIDR_Sym("Print")));           // src: символ → релокация
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("OCL0"),              // rdi
        FUS_HIDR_Imm(30, HIDR_IMM64)));
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_CALL, HIDR_OP_SIZE_64,
        FUS_HIDR_Reg("BCL0"), FUS_HIDR_None()));
    fusInsertCodeBlock(mount, FUS_HIDRM(HIDR_INSTR_RET, HIDR_OP_SIZE_NONE,
        FUS_HIDR_None(), FUS_HIDR_None()));

    // 3. Бэкенд
    FusModuleBackend backend = NULL;
    fusLoaderBackend(instance, &backend, "YOUR_BACKEND", FUS_BACKEND_TYPE_STATIC);

    // 4. Цепочка команд (конфигурация конвейера)
    FusCommandBackend cmd_backend = {
        .sType = FUS_COMMAND_SEND_BACKEND, .backend = backend, .pNext = NULL
    };
    FusCommandHidr cmd_hidr = {
        .sType = FUS_COMMAND_SEND_HIDR,   .code = mount,
        .pNext = (const FusCommandRuleBase_t*)&cmd_backend
    };

    // 5. Генерация
    FusBackendReturn compiler = NULL;
    fusMountHidrsBytes(instance, (FusCommandRuleBase_t*)&cmd_hidr, &compiler);

    // 6. Разрешение символов (если используются)
    FusLinkerContext linker = NULL;
    fusCreateLinkerContext(instance, &linker);
    fusAddSymbolLinker(linker, "Print", (uintptr_t)&Print);
    fusLinkerResolver((FusCommandRuleBase_t*)&cmd_backend, linker, compiler);

    // 7. Исполнение
    FusBufferContext_t* buffer = fusGetStreamBufferCompiler(instance, compiler);
    if (!fusExecutableBuffer(buffer)) {
        typedef void (*Fn)(void);
        ((Fn)buffer->buffer)();
    }

    // 8. Освобождение — всегда, в обратном порядке
    fusDestroyBufferCode(instance, buffer);
    fusDestroyBackendReturn(instance, compiler);
    fusDestroyBackend(instance, backend);
    fusDestroyLinkerContext(instance, linker);
    fusDestroyCodeMount(instance, mount);
    fusDestroyInstance(instance);
    return 0;
}
```

Никакого реестра проходов, никаких флагов оптимизации, никаких дополнительных подготовительных процедур. Восемь шагов, последний из которых — явное освобождение ресурсов.

---

## Концепции

### **HIDR** — представление инструкций в Fusion

Массив узлов. Каждый узел описывает одну инструкцию через `opcode`, размер операндов и два операнда (`dst` и `src`). Никакой системы типов, никакой SSA-формы, никаких проверок. Пользователь объявляет инструкции и их порядок.

```c
FUS_HIDRM(HIDR_INSTR_MOV, HIDR_OP_SIZE_64, FUS_HIDR_Reg("BCL0"), FUS_HIDR_Imm(30, HIDR_IMM64))
```

> **Конструкторы операндов**: `FUS_HIDR_Reg`, `FUS_HIDR_Imm`, `FUS_HIDR_Mem`, `FUS_HIDR_Sym`, `FUS_HIDR_None`.
> **Доступные опкоды**: `MOV`, `ADD`, `CMP`, `ADDR`, `CALL`, `PUSH`, `POP`, `RET`, `SYSCALL`.

### **Символические регистры** — идентификация по роли

HIDR не выставляет идентификаторы регистров архитектуры. Регистры обозначаются своей ролью, и каждый бэкенд сопоставляет её с соответствующим железом.

```
   A C L 0
   │ │ │ └── индекс
   │ │ └──── размер:  L = 64 бита
   │ └────── группа:   C = общего назначения
   └──────── роль:     A = аккумулятор
```

Состав полей, полная таблица символов и примеры сопоставления описаны в **[FusionRegistre](../Documentation/UserDocumentation/FusionRegistre.md)**.

Один и тот же HIDR можно передать на x86, ARM или RISC-V без изменений, поскольку каждый бэкенд переводит объявленную роль в свой набор регистров.

### **Цепочка команд** — конфигурация конвейера

Конвейер настраивается связным списком в стиле `pNext` из Vulkan. Каждая структура объявляет свою роль через `sType` и указывает на следующий элемент. Порядок элементов не имеет значения, поскольку Fusion обходит цепочку целиком.

| `sType` | Структура | Назначение |
|---|---|---|
| `FUS_COMMAND_SEND_BACKEND` | `FusCommandBackend` | Используемый бэкенд |
| `FUS_COMMAND_SEND_HIDR` | `FusCommandHidr` | Входной mount HIDR |
| `FUS_COMMAND_SEND_BUFFER` | `FusCommandBuffer` | Выходной буфер (зарезервировано) |
| `FUS_COMMAND_SEND_LINKER` | — | Зарезервировано |

---

## Справочник API

Публичный API расположен в `include/Fusion/`. Единый заголовок `Fusion/Fusion.h` агрегирует все объявления, при этом каждый заголовок мал и самодостаточен:

| Заголовок | Область |
|---|---|
| `Fusion/FusionTypes.h` | Базовые типы, `FUS_API`, `FUS_DEFINE_HANDLE` |
| `Fusion/FusionInstance.h` | `fusCreateInstance` · `fusDestroyInstance` |
| `Fusion/FusionBuffer.h` | Создание буфера, исполняемое отображение, IO и освобождение |
| `Fusion/FusionCompile.h` | `fusMountHidrsBytes` · `fusGetStreamBufferCompiler` |
| `Fusion/FusionRule.h` | Структуры цепочки команд |
| `Fusion/FusionTrace.h` | Дерево трассировки ошибок |
| `Fusion/Backend/FusionBackend.h` | Загрузка и освобождение бэкендов |
| `Fusion/Linker/FusionLinkerInterface.h` | Символы, секции и разрешение релокаций |
| `Fusion/IRTypes/HidrType.h` | Узлы, операнды и опкоды HIDR |
| `Fusion/IRTypes/HidrHelper.h` | `FusCodeMount` и конструкторы `FUS_HIDR_*` |
| `Fusion/IRTypes/HidrRegistre.h` | Символические регистры |
| `Fusion/IO/FusionGenericIO.h` · `FusionFileIO.h` | Общие и файловые sinks |

Соблюдаемые во всём API соглашения: функции принимают `FusInstance` первым параметром; дескрипторы непрозрачны и объявлены через `FUS_DEFINE_HANDLE`; у каждого созданного ресурса есть соответствующая функция `fusDestroy*`; код возврата — `FUSION_OK` (1) или `FUSION_ERRO` (0), а подробности читаются в дереве трассировки инстанса.

---

## Бэкенды

### Доступный бэкенд

| Бэкенд | Состояние |
|---|---|
| [X86_Backend](https://github.com/Fusion-Toolchain/FusionBackendX86) | Активен — x86-64: `mov` `add` `cmp` `lea` `call` `push` `pop` `ret` `syscall` · релокации `REL32` и `ABS64` |

### Реализация статического бэкенда

Бэкенд реализует две функции и регистрирует себя через макрос:

```c
#include <BackendInterface/Backend.h>

FusBackendInterface_t* MyBackendDefine(void)
{
    static FusBackendInterface_t interface = {
        .FUSI_BackendMountHidrArray   = my_mount_hidr,   // массив HIDR -> блок данных
        .FUSI_BackendLinkerRelocation = my_relocate,    // применяет одну релокацию
    };
    return &interface;
}

REGISTER_BACKEND(My_Backend, MyBackendDefine);
```

Ядро внедряет `FusBackendApi_t`, которая предоставляет выделение памяти, создание выходных блоков, передачи с явным разрушением, регистрацию релокаций и доступ к дереву трассировки. Доступные помощники: `FUSB_ALLOC` `FUSB_FREE` `FUSB_CREATE_BLOCK` `FUSB_CREATE_TRASNFER` `FUSB_REGISTRE_REALOCATION` `FUSB_GET_TRACE_FUSION`.

Тип релокации **непрозрачен** для ядра: его определение и интерпретация принадлежат бэкенду. Ядро не выводит и не изменяет это значение.

---

## Технические детали

- **Собственный аллокатор** — Fusion не вызывает `malloc` напрямую. Передайте `FusInstanceMyAllocation_t` (`Alloc` / `Free` / `Realloc` / `userdata`) или `NULL`, чтобы использовать аллокатор по умолчанию.
- **Дерево трассировки** — каждый внутренний шаг регистрирует узел в дереве ошибок инстанса. `fusDumpTrace` показывает полную цепочку причины и следствия с указанием файла и строки.
- **Управление памятью** — собственные подсистемы Arena, Slab, Handle и LargerBlocks, работающие поверх предоставленного аллокатора.
- **Вывод AOT** — `fusBufferIOSink` записывает байты в любой `FusIOSink`: ELF, плоский бинарный файл или собственный формат.

### Структура каталогов

```
include/Fusion/          # публичный API, subrepo
  Fusion.h               # единый заголовок — агрегирует всё
  FusionTypes.h          # базовые типы, FUS_API, дескрипторы
  FusionInstance.h  FusionBuffer.h  FusionCompile.h  FusionRule.h  FusionTrace.h
  Backend/  Linker/  IRTypes/  IO/
include/BackendInterface/ Backend.h   # API для реализаций бэкендов
include/Internal/                      # приватные заголовки

src/Core/
  Compiler/              # HIDR -> бэкенд -> буфер
  Linker_System/         # символы и разрешение релокаций
  Memory/                # Arena, Slab, Handle, LargerBlocks
  Backend_System/        # регистрация и загрузка бэкендов
  BufferSystem/          # буфер кода и исполняемое отображение
  Error_Tree/  IO_Sytem/  IRTypes/
```

---

## Building

```bash
make -j 6              # собирает libfusion.so
make example           # собирает example_basic и example_program
make -C test test      # запускает тесты под valgrind
```

| Флаг | Эффект |
|---|---|
| `DEBUG=Y` | Включает трассировку `FUSION_DEBUG` в аллокаторах ядра |

```bash
cd example
./example_basic     # JIT: функция C, вызванная сгенерированным кодом
./example_program   # AOT: создаёт output.elf
./output.elf        # → код возврата 1 (exit(60))
```
Требования: компилятор с поддержкой C23 (gcc или clang) и Linux. Единственная зависимость: **libc**.

## Лицензия

GPL-3.0 — см. [LICENSE](../LICENSE).

---

<div align="center">
<sub>Fusion Code Engine — для того, кто хочет <b>генерировать</b> код.</sub>
</div>
