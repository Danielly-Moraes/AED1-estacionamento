#ifndef PILHA_HISTORICO_H
#define PILHA_HISTORICO_H

#include <stddef.h>

#include "veiculo.h"

typedef struct {
    Veiculo veiculo;
    int hora_saida;
    int minutos_permanencia;
    double valor_pago;
} RegistroHistorico;

typedef struct NoHistorico {
    RegistroHistorico registro;
    struct NoHistorico *proximo;
} NoHistorico;

typedef struct {
    NoHistorico *topo;
    size_t qtde;
} PilhaHistorico;

void pilha_inicializar(PilhaHistorico *pilha);
int pilha_empilhar(PilhaHistorico *pilha,
                   const RegistroHistorico *registro);
void pilha_limpar(PilhaHistorico *pilha);

#endif
