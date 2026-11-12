#include "fila_espera.h"

#include <stdlib.h>
#include <string.h>

void fila_inicializar(FilaEspera *fila)
{
    if (fila == NULL) {
        return;
    }
    fila->inicio = NULL;
    fila->fim = NULL;
    fila->qtde = 0;
}

int fila_enfileirar(FilaEspera *fila, const Veiculo *veiculo)
{
    NoFila *novo;

    if (fila == NULL || !veiculo_valido(veiculo)) {
        return 0;
    }
    novo = (NoFila *)malloc(sizeof(NoFila));
    if (novo == NULL) {
        return 0;
    }
    novo->veiculo = *veiculo;
    novo->proximo = NULL;
    if (fila->fim == NULL) {
        fila->inicio = novo;
    } else {
        fila->fim->proximo = novo;
    }
    fila->fim = novo;
    fila->qtde++;
    return 1;
}

int fila_desenfileirar(FilaEspera *fila, Veiculo *veiculo)
{
    NoFila *removido;

    if (fila == NULL || fila->inicio == NULL) {
        return 0;
    }
    removido = fila->inicio;
    if (veiculo != NULL) {
        *veiculo = removido->veiculo;
    }
    fila->inicio = removido->proximo;
    if (fila->inicio == NULL) {
        fila->fim = NULL;
    }
    fila->qtde--;
    free(removido);
    return 1;
}

Veiculo *fila_buscar(FilaEspera *fila, const char *placa)
{
    NoFila *atual;

    if (fila == NULL || placa == NULL) {
        return NULL;
    }
    for (atual = fila->inicio; atual != NULL; atual = atual->proximo) {
        if (strcmp(atual->veiculo.placa, placa) == 0) {
            return &atual->veiculo;
        }
    }
    return NULL;
}

void fila_limpar(FilaEspera *fila)
{
    Veiculo ignorado;

    if (fila == NULL) {
        return;
    }
    while (fila_desenfileirar(fila, &ignorado)) {
    }
}
