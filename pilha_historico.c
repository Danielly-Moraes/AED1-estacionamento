#include "pilha_historico.h"

#include <math.h>
#include <stdlib.h>

void pilha_inicializar(PilhaHistorico *pilha)
{
    if (pilha == NULL) {
        return;
    }
    pilha->topo = NULL;
    pilha->qtde = 0;
}

int pilha_empilhar(PilhaHistorico *pilha,
                   const RegistroHistorico *registro)
{
    NoHistorico *novo;

    if (pilha == NULL || registro == NULL ||
        !veiculo_valido(&registro->veiculo) ||
        registro->hora_saida < 0 || registro->hora_saida >= 24 * 60 ||
        registro->minutos_permanencia < 0 ||
        !isfinite(registro->valor_pago) || registro->valor_pago < 0.0) {
        return 0;
    }
    novo = (NoHistorico *)malloc(sizeof(NoHistorico));
    if (novo == NULL) {
        return 0;
    }
    novo->registro = *registro;
    novo->proximo = pilha->topo;
    pilha->topo = novo;
    pilha->qtde++;
    return 1;
}

void pilha_limpar(PilhaHistorico *pilha)
{
    NoHistorico *atual;

    if (pilha == NULL) {
        return;
    }
    atual = pilha->topo;
    while (atual != NULL) {
        NoHistorico *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    pilha->topo = NULL;
    pilha->qtde = 0;
}
