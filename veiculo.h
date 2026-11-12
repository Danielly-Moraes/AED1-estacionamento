#ifndef VEICULO_H
#define VEICULO_H

#define TAM_PLACA 10
#define TAM_MODELO 30
#define TAM_COR 20

typedef struct {
    char placa[TAM_PLACA];
    char modelo[TAM_MODELO];
    char cor[TAM_COR];
    int hora_entrada;
    int tipo;
} Veiculo;

int veiculo_hora_atual(int *minutos);
void veiculo_formatar_hora(int minutos, char *destino, int tamanho);
int veiculo_placa_valida(const char *placa);
int veiculo_valido(const Veiculo *veiculo);

#endif
