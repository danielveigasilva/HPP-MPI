# HPP-MPI

## Execução

```
mpirun -np [NI] --oversubscribe ./bin/HPP-MPI [N] [NP] [I] [MS]
```

<kbd>NI</kbd> : Quantidade de processos para execução paralela.

<kbd>N</kbd> : Tamanho da matriz NxN.

<kbd>NP</kbd> : Quantidade de partículas.

<kbd>I</kbd> : Quantidade de interações.

<kbd>MS</kbd> (Opcional): Taxa de atualização em milisegundos. Se informado produz uma animação no terminal. Caso seja omitido realiza a comparação da execução paralela e sequêncial.

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
