#ifndef LISTA_ESTACIONAMENTO_H
#define LISTA_ESTACIONAMENTO_H

#include <stddef.h>

#include "veiculo.h"

typedef struct NoEstacionamento {
    Veiculo veiculo;
    struct NoEstacionamento *proximo;
} NoEstacionamento;

typedef struct {
    NoEstacionamento *inicio;
    NoEstacionamento *fim;
    size_t qtde;
    size_t capacidade_maxima;
} ListaEstacionamento;

void lista_inicializar(ListaEstacionamento *lista, size_t capacidade);
int lista_inserir(ListaEstacionamento *lista, const Veiculo *veiculo);
Veiculo *lista_buscar(ListaEstacionamento *lista, const char *placa);
int lista_remover(ListaEstacionamento *lista, const char *placa,
                  Veiculo *removido);
int lista_contem(ListaEstacionamento *lista, const char *placa);
void lista_limpar(ListaEstacionamento *lista);

#endif
