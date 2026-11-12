#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include "fila_espera.h"
#include "lista_estacionamento.h"
#include "pilha_historico.h"

typedef struct {
    double tarifa_primeira_hora;
    double tarifa_hora_adicional;
} Tarifas;

int persistencia_salvar_patio(const ListaEstacionamento *lista,
                              const Tarifas *tarifas, const char *caminho);
int persistencia_carregar_patio(ListaEstacionamento *lista, Tarifas *tarifas,
                                const char *caminho);
int persistencia_salvar_fila(const FilaEspera *fila, const char *caminho);
int persistencia_carregar_fila(FilaEspera *fila, const char *caminho);
int persistencia_salvar_historico(const PilhaHistorico *pilha,
                                  const char *caminho);
int persistencia_carregar_historico(PilhaHistorico *pilha, const char *caminho);

#endif
