#include "veiculo.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int veiculo_hora_atual(int *minutos)
{
    time_t agora;
    struct tm *horario;

    if (minutos == NULL) {
        return 0;
    }

    agora = time(NULL);
    horario = localtime(&agora);
    if (horario == NULL) {
        return 0;
    }

    *minutos = horario->tm_hour * 60 + horario->tm_min;
    return 1;
}

void veiculo_formatar_hora(int minutos, char *destino, int tamanho)
{
    if (destino == NULL || tamanho <= 0) {
        return;
    }
    if (minutos < 0 || minutos >= 24 * 60) {
        snprintf(destino, (size_t)tamanho, "--:--");
        return;
    }
    snprintf(destino, (size_t)tamanho, "%02d:%02d",
             minutos / 60, minutos % 60);
}

int veiculo_placa_valida(const char *placa)
{
    size_t i;
    size_t tamanho;

    if (placa == NULL) {
        return 0;
    }
    tamanho = strlen(placa);
    if (tamanho < 1 || tamanho >= TAM_PLACA) {
        return 0;
    }
    for (i = 0; i < tamanho; i++) {
        if (!isalnum((unsigned char)placa[i]) && placa[i] != '-') {
            return 0;
        }
    }
    return 1;
}

int veiculo_valido(const Veiculo *veiculo)
{
    if (veiculo == NULL ||
        memchr(veiculo->placa, '\0', sizeof(veiculo->placa)) == NULL ||
        memchr(veiculo->modelo, '\0', sizeof(veiculo->modelo)) == NULL ||
        memchr(veiculo->cor, '\0', sizeof(veiculo->cor)) == NULL ||
        !veiculo_placa_valida(veiculo->placa) ||
        veiculo->modelo[0] == '\0' ||
        veiculo->cor[0] == '\0' ||
        veiculo->hora_entrada < 0 ||
        veiculo->hora_entrada >= 24 * 60 ||
        (veiculo->tipo != 1 && veiculo->tipo != 2)) {
        return 0;
    }
    return 1;
}
