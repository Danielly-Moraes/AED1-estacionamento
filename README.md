# Controle de Estacionamento

Projeto acadêmico de AED1 em C99, organizado em TADs: lista encadeada para o
pátio, fila encadeada para espera e pilha encadeada para histórico.

## Compilação e execução

Com GCC e Make:

```sh
make
./estacionamento
```

Ou compile diretamente:

```sh
gcc -std=c99 -Wall -Wextra -pedantic -O2 main.c veiculo.c lista_estacionamento.c fila_espera.c pilha_historico.c persistencia.c -o estacionamento
```

O programa cria/carrega `patio.dat`, `fila.dat` e `historico.dat` no diretório
de execução. Esses arquivos binários devem permanecer junto ao executável para
preservar o estado entre execuções. A tarifa padrão é R$ 10,00 pela primeira
hora e R$ 5,00 por hora adicional; ambas podem ser alteradas no menu.
