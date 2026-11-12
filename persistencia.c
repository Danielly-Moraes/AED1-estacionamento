#include "persistencia.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char assinatura[8];
    uint32_t versao;
    uint32_t quantidade;
    uint32_t capacidade;
    double tarifa_primeira_hora;
    double tarifa_hora_adicional;
} CabecalhoPatio;

typedef struct {
    char assinatura[8];
    uint32_t versao;
    uint32_t quantidade;
} CabecalhoSimples;

static int salvar_lista(FILE *arquivo, const ListaEstacionamento *lista)
{
    const NoEstacionamento *atual;

    for (atual = lista->inicio; atual != NULL; atual = atual->proximo) {
        if (fwrite(&atual->veiculo, sizeof(Veiculo), 1, arquivo) != 1) {
            return 0;
        }
    }
    return 1;
}

int persistencia_salvar_patio(const ListaEstacionamento *lista,
                              const Tarifas *tarifas, const char *caminho)
{
    FILE *arquivo;
    CabecalhoPatio cabecalho;
    int ok;

    if (lista == NULL || tarifas == NULL || caminho == NULL ||
        lista->qtde > UINT32_MAX || lista->capacidade_maxima > UINT32_MAX) {
        return 0;
    }
    arquivo = fopen(caminho, "wb");
    if (arquivo == NULL) {
        return 0;
    }
    memset(&cabecalho, 0, sizeof(cabecalho));
    memcpy(cabecalho.assinatura, "PATIO01", 7);
    cabecalho.versao = 1;
    cabecalho.quantidade = (uint32_t)lista->qtde;
    cabecalho.capacidade = (uint32_t)lista->capacidade_maxima;
    cabecalho.tarifa_primeira_hora = tarifas->tarifa_primeira_hora;
    cabecalho.tarifa_hora_adicional = tarifas->tarifa_hora_adicional;
    ok = fwrite(&cabecalho, sizeof(cabecalho), 1, arquivo) == 1 &&
         salvar_lista(arquivo, lista);
    if (fclose(arquivo) != 0) {
        ok = 0;
    }
    return ok;
}

int persistencia_carregar_patio(ListaEstacionamento *lista, Tarifas *tarifas,
                                const char *caminho)
{
    FILE *arquivo;
    CabecalhoPatio cabecalho;
    uint32_t i;

    if (lista == NULL || tarifas == NULL || caminho == NULL) {
        return -1;
    }
    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        return errno == ENOENT ? 0 : -1;
    }
    if (fread(&cabecalho, sizeof(cabecalho), 1, arquivo) != 1 ||
        memcmp(cabecalho.assinatura, "PATIO01", 7) != 0 ||
        cabecalho.versao != 1 || cabecalho.capacidade == 0 ||
        cabecalho.quantidade > cabecalho.capacidade ||
        !isfinite(cabecalho.tarifa_primeira_hora) ||
        !isfinite(cabecalho.tarifa_hora_adicional) ||
        cabecalho.tarifa_primeira_hora < 0.0 ||
        cabecalho.tarifa_hora_adicional < 0.0) {
        fclose(arquivo);
        return -1;
    }
    lista->capacidade_maxima = cabecalho.capacidade;
    tarifas->tarifa_primeira_hora = cabecalho.tarifa_primeira_hora;
    tarifas->tarifa_hora_adicional = cabecalho.tarifa_hora_adicional;
    for (i = 0; i < cabecalho.quantidade; i++) {
        Veiculo veiculo;
        if (fread(&veiculo, sizeof(veiculo), 1, arquivo) != 1 ||
            lista_inserir(lista, &veiculo) != 1) {
            lista_limpar(lista);
            fclose(arquivo);
            return -1;
        }
    }
    if (fgetc(arquivo) != EOF) {
        lista_limpar(lista);
        fclose(arquivo);
        return -1;
    }
    if (ferror(arquivo)) {
        lista_limpar(lista);
        fclose(arquivo);
        return -1;
    }
    fclose(arquivo);
    return 1;
}

int persistencia_salvar_fila(const FilaEspera *fila, const char *caminho)
{
    FILE *arquivo;
    CabecalhoSimples cabecalho;
    const NoFila *atual;
    int ok = 1;

    if (fila == NULL || caminho == NULL || fila->qtde > UINT32_MAX) {
        return 0;
    }
    arquivo = fopen(caminho, "wb");
    if (arquivo == NULL) {
        return 0;
    }
    memset(&cabecalho, 0, sizeof(cabecalho));
    memcpy(cabecalho.assinatura, "FILA0001", 8);
    cabecalho.versao = 1;
    cabecalho.quantidade = (uint32_t)fila->qtde;
    if (fwrite(&cabecalho, sizeof(cabecalho), 1, arquivo) != 1) {
        ok = 0;
    }
    for (atual = fila->inicio; ok && atual != NULL; atual = atual->proximo) {
        if (fwrite(&atual->veiculo, sizeof(Veiculo), 1, arquivo) != 1) {
            ok = 0;
        }
    }
    if (fclose(arquivo) != 0) {
        ok = 0;
    }
    return ok;
}

int persistencia_carregar_fila(FilaEspera *fila, const char *caminho)
{
    FILE *arquivo;
    CabecalhoSimples cabecalho;
    uint32_t i;

    if (fila == NULL || caminho == NULL) {
        return -1;
    }
    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        return errno == ENOENT ? 0 : -1;
    }
    if (fread(&cabecalho, sizeof(cabecalho), 1, arquivo) != 1 ||
        memcmp(cabecalho.assinatura, "FILA0001", 8) != 0 ||
        cabecalho.versao != 1) {
        fclose(arquivo);
        return -1;
    }
    for (i = 0; i < cabecalho.quantidade; i++) {
        Veiculo veiculo;
        if (fread(&veiculo, sizeof(veiculo), 1, arquivo) != 1 ||
            !fila_enfileirar(fila, &veiculo)) {
            fila_limpar(fila);
            fclose(arquivo);
            return -1;
        }
    }
    if (fgetc(arquivo) != EOF || ferror(arquivo)) {
        fila_limpar(fila);
        fclose(arquivo);
        return -1;
    }
    fclose(arquivo);
    return 1;
}

int persistencia_salvar_historico(const PilhaHistorico *pilha,
                                  const char *caminho)
{
    FILE *arquivo;
    CabecalhoSimples cabecalho;
    const NoHistorico *atual;
    int ok = 1;

    if (pilha == NULL || caminho == NULL || pilha->qtde > UINT32_MAX) {
        return 0;
    }
    arquivo = fopen(caminho, "wb");
    if (arquivo == NULL) {
        return 0;
    }
    memset(&cabecalho, 0, sizeof(cabecalho));
    memcpy(cabecalho.assinatura, "HIST0001", 8);
    cabecalho.versao = 1;
    cabecalho.quantidade = (uint32_t)pilha->qtde;
    if (fwrite(&cabecalho, sizeof(cabecalho), 1, arquivo) != 1) {
        ok = 0;
    }
    for (atual = pilha->topo; ok && atual != NULL; atual = atual->proximo) {
        if (fwrite(&atual->registro, sizeof(RegistroHistorico), 1, arquivo) != 1) {
            ok = 0;
        }
    }
    if (fclose(arquivo) != 0) {
        ok = 0;
    }
    return ok;
}

int persistencia_carregar_historico(PilhaHistorico *pilha, const char *caminho)
{
    FILE *arquivo;
    CabecalhoSimples cabecalho;
    RegistroHistorico *registros;
    uint32_t i;

    if (pilha == NULL || caminho == NULL) {
        return -1;
    }
    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        return errno == ENOENT ? 0 : -1;
    }
    if (fread(&cabecalho, sizeof(cabecalho), 1, arquivo) != 1 ||
        memcmp(cabecalho.assinatura, "HIST0001", 8) != 0 ||
        cabecalho.versao != 1) {
        fclose(arquivo);
        return -1;
    }
    registros = NULL;
    if (cabecalho.quantidade > 0) {
        registros = (RegistroHistorico *)malloc(
            (size_t)cabecalho.quantidade * sizeof(RegistroHistorico));
        if (registros == NULL) {
            fclose(arquivo);
            return -1;
        }
    }
    for (i = 0; i < cabecalho.quantidade; i++) {
        if (fread(&registros[i], sizeof(RegistroHistorico), 1, arquivo) != 1) {
            free(registros);
            fclose(arquivo);
            return -1;
        }
        if (!veiculo_valido(&registros[i].veiculo) ||
            registros[i].hora_saida < 0 ||
            registros[i].hora_saida >= 24 * 60 ||
            registros[i].minutos_permanencia < 0 ||
            !isfinite(registros[i].valor_pago) ||
            registros[i].valor_pago < 0.0) {
            free(registros);
            fclose(arquivo);
            return -1;
        }
    }
    if (fgetc(arquivo) != EOF || ferror(arquivo)) {
        free(registros);
        fclose(arquivo);
        return -1;
    }
    fclose(arquivo);
    for (i = cabecalho.quantidade; i > 0; i--) {
        if (!pilha_empilhar(pilha, &registros[i - 1])) {
            free(registros);
            pilha_limpar(pilha);
            return -1;
        }
    }
    free(registros);
    return 1;
}
