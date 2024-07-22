# HPP-MPI
## Compilação
```
mpicc -o HPP-MPI HPP_MPI.c
```
## Execução

```
mpirun -np [NI] --oversubscribe ./HPP-MPI [N] [NP] [I] [MS]
```

<kbd>NI</kbd> : Quantidade de processos para execução paralela.

<kbd>N</kbd> : Tamanho da matriz NxN.

<kbd>NP</kbd> : Quantidade e partículas.

<kbd>I</kbd> : Quantidade de interações.

<kbd>MS</kbd> (Opcional): Se informado produz uma animação no terminal com uma taxa de atualização em milisegundos. Caso seja omitido realiza a comparação da execução paralela e sequêncial.

### Exemplo 1

```
mpirun -np 8 --oversubscribe ./HPP-MPI 40 200 100
```
<p align="center">
    <img src="/imagens/exemplo2.png" width="80%">
</p>

### Exemplo 2

```
mpirun -np 8 --oversubscribe ./HPP-MPI 40 200 100 50000
```
<p align="center">
    <img src="/imagens/exemplo1.gif" width="80%">
</p>
