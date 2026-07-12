# Fusion — Convenções de Nomenclatura

Inspirado no Vulkan. A primeira letra do prefixo já diz o que é:

| O quê         | Prefixo   | Exemplo                          |
|---------------|-----------|-----------------------------------|
| Tipo / handle | `Fus` (P) | `FusInstance`, `FusTraceTree_t`   |
| Função pública| `fus` (m) | `fusCreateInstance`               |
| Função interna| `fusi`(m) | `fusiPushError`                   |
| Macro / enum  | `FUS_`    | `FUS_ERR_INTENT_IMPOSSIBLE`       |

Regra de bolso: **maiúscula = tipo, minúscula = função**. Igual `VkInstance` vs `vkCreateInstance`.

## Handles (ponteiros opacos)

Sempre via `FUS_DEFINE_HANDLE(Nome)` — nunca escrever `_t` na mão, a macro já adiciona.

- Função que **cria** o handle → recebe `TipoHandle*` (out-param, escreve `*out`)
- Função que só **usa** o handle → recebe `TipoHandle` (sem `*`)

> Se aparecer `struct X_T**` num erro de compilação, é esse o primeiro lugar a checar.

## Status quo (histórico, não migrar agora)

Código já existente continua `FUS_CreateInstance` etc. — só **código novo** nasce
no padrão `fus`/`Fus`/`fusi`. Migração em massa fica pra uma sessão dedicada,
com tempo de rodar os testes com calma depois.
