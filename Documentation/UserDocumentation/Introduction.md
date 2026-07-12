# Fusion — Engine de Geração de Código

## O Problema

Compiladores e geradores de código modernos operam sob uma premissa implícita: **o gerador possui autoridade sobre o resultado**.
Flags de otimização como `-O3` delegam ao compilador decisões que, em muitos contextos, pertencem ao domínio do engenheiro — granularidade de otimização, seleção de instruções, trade-offs entre latência de compilação e qualidade de emissão.
Para sistemas que exigem controle preciso sobre geração — compiladores de linguagens, runtimes, geradores de código de domínio específico — essa arquitetura impõe limitações fundamentais. O usuário opera sobre abstrações que escondem o mecanismo real, sem acesso direto às decisões de geração.
O Fusion parte de uma premissa oposta: **o controle pertence a quem conhece o domínio**. A engine não toma decisões pelo usuário — ela fornece a infraestrutura para que decisões precisas sejam expressas, compostas e executadas.
