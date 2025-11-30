## TRABALHO 2: ANÁLISE COMPARATIVA DE ESTRUTURAS DE DADOS
Este repositório contém a implementação final em C++ para o Trabalho 2 da disciplina de **Projeto e Análise de Algoritmos (PAA)**. O objetivo foi realizar uma análise de custos computacionais de quatro estruturas para busca de similaridade em alta dimensão (Histogramas RGB 512D), com foco na inclusão e avaliação da M-Tree.

---
## Status Final do Projeto
O código-base está completo, funcional e foi avaliado com sucesso utilizando bases de dados de até N=100 imagens.

**Estruturas de Dados Implementadas**

O sistema integra e compara quatro metodologias distintas de indexação, todas operando com histogramas de cor normalizados de 512 dimensões e Distância Qui-quadrado.
```
 _________________________________________________________________________________________________________
|   Estrutura    |          Propósito            | Complexidade de Busca  |          Tipo de Busca        |
|________________|_______________________________|________________________|_______________________________|
|     Lista      |            Baseline           |       O (N · D)        |             Exata             |
| Hash (SimHash) |           Aproximação         | Sublinear / Quase O(1) |          Aproximada           |
|    Quadtree    | Particionamento Espacial (2D) |   O(log N) esperado    | Exata (com perda de precisão) |
|     M-Tree     |         Métrica, Alta D       |  O(log N · D) esperado |             Exata             |
|_________________________________________________________________________________________________________|
```
---
## 1. Representação dos Dados

* **Descritor:** Histograma RGB de 8x8x8 bins, resultando em um vetor de 512 dimensões.

* **Métrica Exata: Distância Qui-quadrado** (usada por Lista, Quadtree e M-Tree) para medir similaridade entre histogramas.

* **Análise:** O main.cpp inclui medições de tempo de Construção e Busca (std::chrono) para a avaliação empírica de custos.

## 2. Observações Cruciais sobre a M-Tree

A implementação da **M-Tree** em *search_mtree.hpp* foi crucial para a análise de custos, pois forneceu a única busca exata em tempo sublinear na métrica Qui-quadrado.

Apesar de ser funcional, a M-Tree implementa uma versão simplificada:

* **Heurística de Split:** A divisão do nó é feita utilizando uma *heurística de promoção básica* (ex: último elemento) e não algoritmos avançados como MinMax ou Balanced Redistribution.
* **Implicação:** Esta simplificação **não compromete a exatidão** da busca (o resultado 1-NN é sempre correto), mas é uma simplificação de engenharia que deve ser considerada ao analisar o custo de **construção** *(O(N log N))*, que seria otimizado em uma versão formal para escala maior.

O relatório final utiliza a superioridade da busca M-Tree ($O(\log N)$) em relação à Lista ($O(N)$) e a sua exatidão (em contraste com a Quadtree) para justificar a escolha da estrutura.
