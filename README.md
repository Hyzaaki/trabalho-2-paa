## Status do Projeto
O código já está funcionando com sucesso para as quatro estruturas:
* **Lista** (Linear Search)
* **Hash** (SimHash 128b + Hamming)
* **Quadtree** (2D baseado no histograma RGB)
* **M-Tree** (estrutura métrica simplificada)
Também já está implementado:
* **cálculo de histograma RGB** (8×8×8 → 512 bins)
* **distância qui-quadrado**
* **busca 1-NN para todas as estruturas**
* **medição de tempo de construção e busca** (com chrono)
* **comparativo de tempos no final da execução**

---
## O que ainda falta fazer
1. **Usar mais imagens nos experimentos**
* Atualmente o teste usa apenas 4 imagens.
* O ideal para o relatório é entre 50 e 200 imagens
* todas no formato PPM P6 (o código só aceita esse formato)

2. **Ajustar o main para testes em lote**

3. **Gerar tabelas para o relatório**
Depois que o código estiver rodando com N imagens, será necessário produzir:
* tabela de tempo de construção das 4 estruturas
* tabela de tempo de busca
* comparação conceitual (teórica vs prática)
* análise dos resultados
* observações sobre limitações da implementação

---
## Observações Importantes sobre a M-Tree (para o relatório)
A implementação atual funciona perfeitamente para 1-NN, mas:
* usa um split simplificado
* não segue 100% o algoritmo formal de Ciaccia et al. (1997)
* o nó que divide gera um novo nó irmão, mas não faz promoção ideal de pivô
* para o Trabalho 2, isso é aceitável, desde que explicado no relatório

---
## Pontos importantes para mencionar no relatório

**A M-Tree implementada é funcional, mas não usa heurísticas avançadas**
A implementação usa:
* promoção simples (último elemento)
* split básico
* sem minMax, balanced ou random promotion

**Isso NÃO afeta as buscas 1-NN nesse conjunto pequeno**
O resultado continuará o mesmo que a lista linear.

**A análise teórica continua válida**
Mesmo com split simples, a M-Tree:
* é uma estrutura métrica
* usa distância qui-quadrado
* permite poda de regiões
* tem custo assintótico sublinear esperado

**É válido explicar no relatório que uma versão simplificada foi usada**

**Se você quiser no futuro: Implementar um split correto (opcional)**
Uma versão completa da M-Tree deveria implementar:
* promoção de pivô por heurística minMax
* redistribuição equilibrada dos itens
* propagação do split para cima (pai que faz o split, não a folha)
* função separada trySplit(child)
* Mas não é obrigatório para o trabalho.