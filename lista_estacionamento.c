#include "lista_estacionamento.h"

#include <stdlib.h>
#include <string.h>

void lista_inicializar(ListaEstacionamento *lista, size_t capacidade)
{
    if (lista == NULL) {
        return;
    }
    lista->inicio = NULL;
    lista->fim = NULL;
    lista->qtde = 0;
    lista->capacidade_maxima = capacidade;
}

Veiculo *lista_buscar(ListaEstacionamento *lista, const char *placa)
{
    NoEstacionamento *atual;

    if (lista == NULL || placa == NULL) {
        return NULL;
    }
    for (atual = lista->inicio; atual != NULL; atual = atual->proximo) {
        if (strcmp(atual->veiculo.placa, placa) == 0) {
            return &atual->veiculo;
        }
    }
    return NULL;
}

int lista_contem(ListaEstacionamento *lista, const char *placa)
{
    return lista_buscar(lista, placa) != NULL;
}

int lista_inserir(ListaEstacionamento *lista, const Veiculo *veiculo)
{
    NoEstacionamento *novo;

    if (lista == NULL || !veiculo_valido(veiculo)) {
        return -1;
    }
    if (lista->qtde >= lista->capacidade_maxima) {
        return 0;
    }
    if (lista_contem(lista, veiculo->placa)) {
        return -1;
    }

    novo = (NoEstacionamento *)malloc(sizeof(NoEstacionamento));
    if (novo == NULL) {
        return -1;
    }
    novo->veiculo = *veiculo;
    novo->proximo = NULL;
    if (lista->fim == NULL) {
        lista->inicio = novo;
    } else {
        lista->fim->proximo = novo;
    }
    lista->fim = novo;
    lista->qtde++;
    return 1;
}

int lista_remover(ListaEstacionamento *lista, const char *placa,
                  Veiculo *removido)
{
    NoEstacionamento *atual;
    NoEstacionamento *anterior = NULL;

    if (lista == NULL || placa == NULL) {
        return 0;
    }
    atual = lista->inicio;
    while (atual != NULL && strcmp(atual->veiculo.placa, placa) != 0) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (atual == NULL) {
        return 0;
    }
    if (removido != NULL) {
        *removido = atual->veiculo;
    }
    if (anterior == NULL) {
        lista->inicio = atual->proximo;
    } else {
        anterior->proximo = atual->proximo;
    }
    if (lista->fim == atual) {
        lista->fim = anterior;
    }
    free(atual);
    lista->qtde--;
    return 1;
}

void lista_limpar(ListaEstacionamento *lista)
{
    NoEstacionamento *atual;

    if (lista == NULL) {
        return;
    }
    atual = lista->inicio;
    while (atual != NULL) {
        NoEstacionamento *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    lista->inicio = NULL;
    lista->fim = NULL;
    lista->qtde = 0;
}
