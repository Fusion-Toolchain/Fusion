# HIDR Register String

## Tabela de Caracteres

### ROLE
| Char | Significado |
|------|-------------|
| `A`  | ACC     — Acumulador      |
| `B`  | BASE    — Base            |
| `T`  | COUNTER — Contador        |
| `D`  | DATA    — Dado geral      |
| `S`  | SP      — Stack Pointer   |
| `P`  | BP      — Base Pointer    |
| `I`  | SRC     — Source          |
| `O`  | DST     — Destination     |
| `K`  | LINK    — Link Register   |
| `R`  | RET     — Retorno         |
| `G`  | ARG     — Argumento       |
| `M`  | TMP     — Temporário      |

### GROUP
| Char | Significado |
|------|-------------|
| `C`  | GP    — General Purpose  |
| `F`  | FLOAT — Ponto Flutuante  |
| `V`  | SIMD  — Vetorial         |
| `X`  | CTRL  — Controle         |
| `E`  | SEG   — Segmento         |
| `Z`  | DEBUG — Debug            |

### SIZE
| Char | Significado |
|------|-------------|
| `N`  | 8   bits         |
| `W`  | 16  bits         |
| `H`  | 32  bits         |
| `L`  | 64  bits         |
| `Q`  | 128 bits (SIMD)  |
| `U`  | 256 bits (AVX)   |
| `Y`  | 512 bits (AVX-512) |

### INDEX
| Valor | Significado |
|-------|-------------|
| `0-N` | Índice dentro da categoria |

## Exemplos
| String  | Registrador x86 |
|---------|-----------------|
| `ACL0`  | rax             |
| `ACH0`  | eax             |
| `BCL0`  | rbx             |
| `TCL0`  | rcx             |
| `SCL0`  | rsp             |
| `PCL0`  | rbp             |
| `ICL0`  | rsi             |
| `OCL0`  | rdi             |