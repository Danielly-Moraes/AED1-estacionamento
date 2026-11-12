#ifndef FILA_ESPERA_H
#define FILA_ESPERA_H

#include <stddef.h>

#include "veiculo.h"

typedef struct NoFila {
    Veiculo veiculo;
    struct NoFila *proximo;
} NoFila;

typedef struct {
    NoFila *inicio;
    NoFila *fim;
    size_t qtde;
} FilaEspera;

void fila_inicializar(FilaEspera *fila);
int fila_enfileirar(FilaEspera *fila, const Veiculo *veiculo);
int fila_desenfileirar(FilaEspera *fila, Veiculo *veiculo);
Veiculo *fila_buscar(FilaEspera *fila, const char *placa);
void fila_limpar(FilaEspera *fila);

#endif
