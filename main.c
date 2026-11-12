#include "fila_espera.h"
#include "lista_estacionamento.h"
#include "persistencia.h"
#include "pilha_historico.h"
#include "veiculo.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO_PATIO "patio.dat"
#define ARQUIVO_FILA "fila.dat"
#define ARQUIVO_HISTORICO "historico.dat"

static int ler_linha(const char *mensagem, char *destino, size_t tamanho)
{
    size_t comprimento;
    int caractere;

    printf("%s", mensagem);
    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        return 0;
    }
    comprimento = strlen(destino);
    if (comprimento > 0 && destino[comprimento - 1] == '\n') {
        destino[--comprimento] = '\0';
    } else if (comprimento == tamanho - 1) {
        caractere = getchar();
        if (caractere != '\n' && caractere != EOF) {
            while (caractere != '\n' && caractere != EOF) {
                caractere = getchar();
            }
            destino[0] = '\0';
            puts("Entrada muito longa; tente novamente.");
            return -1;
        }
    }
    while (comprimento > 0 && isspace((unsigned char)destino[comprimento - 1])) {
        destino[--comprimento] = '\0';
    }
    return 1;
}

static int ler_inteiro(const char *mensagem, int *valor)
{
    char linha[80];
    char *fim;
    long numero;
    int resultado;

    resultado = ler_linha(mensagem, linha, sizeof(linha));
    if (resultado != 1) {
        return resultado;
    }
    errno = 0;
    numero = strtol(linha, &fim, 10);
    while (isspace((unsigned char)*fim)) {
        fim++;
    }
    if (linha[0] == '\0' || *fim != '\0' || errno == ERANGE ||
        numero < -2147483647L - 1L || numero > 2147483647L) {
        return -1;
    }
    *valor = (int)numero;
    return 1;
}

static int ler_decimal(const char *mensagem, double *valor)
{
    char linha[80];
    char *fim;
    double numero;
    int resultado;

    resultado = ler_linha(mensagem, linha, sizeof(linha));
    if (resultado != 1) {
        return resultado;
    }
    errno = 0;
    numero = strtod(linha, &fim);
    while (isspace((unsigned char)*fim)) {
        fim++;
    }
    if (linha[0] == '\0' || *fim != '\0' || errno == ERANGE ||
        !isfinite(numero) || numero < 0.0) {
        return -1;
    }
    *valor = numero;
    return 1;
}

static int ler_placa(char placa[TAM_PLACA])
{
    size_t i;
    int resultado;

    resultado = ler_linha("Placa: ", placa, TAM_PLACA);
    if (resultado != 1) {
        return resultado;
    }
    for (i = 0; placa[i] != '\0'; i++) {
        placa[i] = (char)toupper((unsigned char)placa[i]);
    }
    return veiculo_placa_valida(placa) ? 1 : -1;
}

static int salvar_estado(const ListaEstacionamento *lista,
                         const FilaEspera *fila,
                         const PilhaHistorico *historico,
                         const Tarifas *tarifas)
{
    int ok = 1;

    if (!persistencia_salvar_patio(lista, tarifas, ARQUIVO_PATIO)) {
        fprintf(stderr, "Erro ao salvar %s.\n", ARQUIVO_PATIO);
        ok = 0;
    }
    if (!persistencia_salvar_fila(fila, ARQUIVO_FILA)) {
        fprintf(stderr, "Erro ao salvar %s.\n", ARQUIVO_FILA);
        ok = 0;
    }
    if (!persistencia_salvar_historico(historico, ARQUIVO_HISTORICO)) {
        fprintf(stderr, "Erro ao salvar %s.\n", ARQUIVO_HISTORICO);
        ok = 0;
    }
    return ok;
}

static int veiculo_em_uso(const ListaEstacionamento *lista,
                          const FilaEspera *fila, const char *placa)
{
    const NoEstacionamento *no_lista;
    const NoFila *no_fila;

    for (no_lista = lista->inicio; no_lista != NULL; no_lista = no_lista->proximo) {
        if (strcmp(no_lista->veiculo.placa, placa) == 0) {
            return 1;
        }
    }
    for (no_fila = fila->inicio; no_fila != NULL; no_fila = no_fila->proximo) {
        if (strcmp(no_fila->veiculo.placa, placa) == 0) {
            return 1;
        }
    }
    return 0;
}

static void registrar_entrada(ListaEstacionamento *lista, FilaEspera *fila)
{
    Veiculo veiculo;
    int resposta;
    int opcao;

    memset(&veiculo, 0, sizeof(veiculo));
    resposta = ler_placa(veiculo.placa);
    if (resposta == 0) {
        return;
    }
    if (resposta < 0) {
        puts("Placa invalida (use ate 9 letras, numeros ou hifen).");
        return;
    }
    if (veiculo_em_uso(lista, fila, veiculo.placa)) {
        puts("Ja existe um veiculo com essa placa no patio ou na fila.");
        return;
    }
    if (ler_linha("Modelo: ", veiculo.modelo, sizeof(veiculo.modelo)) != 1 ||
        veiculo.modelo[0] == '\0') {
        puts("Modelo invalido.");
        return;
    }
    if (ler_linha("Cor: ", veiculo.cor, sizeof(veiculo.cor)) != 1 ||
        veiculo.cor[0] == '\0') {
        puts("Cor invalida.");
        return;
    }
    resposta = ler_inteiro("Tipo (1-Carro, 2-Moto): ", &opcao);
    if (resposta == 0) {
        return;
    }
    if (resposta < 0 || (opcao != 1 && opcao != 2)) {
        puts("Tipo invalido.");
        return;
    }
    veiculo.tipo = opcao;
    if (!veiculo_hora_atual(&veiculo.hora_entrada)) {
        puts("Nao foi possivel obter o horario atual.");
        return;
    }

    if (lista->qtde < lista->capacidade_maxima) {
        if (lista_inserir(lista, &veiculo) != 1) {
            puts("Nao foi possivel registrar o veiculo (memoria ou dados).");
            return;
        }
        puts("Entrada registrada no patio.");
    } else {
        resposta = ler_inteiro("Patio cheio. Deseja entrar na fila? (1-Sim, 0-Nao): ",
                               &opcao);
        if (resposta == 0) {
            return;
        }
        if (resposta < 0 || (opcao != 0 && opcao != 1)) {
            puts("Opcao invalida; veiculo nao registrado.");
            return;
        }
        if (opcao == 0) {
            puts("Veiculo nao registrado.");
            return;
        }
        if (!fila_enfileirar(fila, &veiculo)) {
            puts("Nao foi possivel adicionar a fila (memoria ou dados).");
            return;
        }
        puts("Veiculo adicionado ao fim da fila de espera.");
    }
}

static void registrar_saida(ListaEstacionamento *lista, FilaEspera *fila,
                            PilhaHistorico *historico, const Tarifas *tarifas)
{
    char placa[TAM_PLACA];
    Veiculo removido;
    RegistroHistorico registro;
    int resposta;
    int agora;
    int horas;

    resposta = ler_placa(placa);
    if (resposta == 0) {
        return;
    }
    if (resposta < 0) {
        puts("Placa invalida.");
        return;
    }
    if (!veiculo_hora_atual(&agora)) {
        puts("Nao foi possivel obter o horario atual.");
        return;
    }
    if (!lista_remover(lista, placa, &removido)) {
        puts("Veiculo nao encontrado no patio.");
        return;
    }

    registro.veiculo = removido;
    registro.hora_saida = agora;
    registro.minutos_permanencia = (agora - removido.hora_entrada + 24 * 60)
                                   % (24 * 60);
    horas = (registro.minutos_permanencia + 59) / 60;
    if (horas < 1) {
        horas = 1;
    }
    registro.valor_pago = tarifas->tarifa_primeira_hora +
        (horas - 1) * tarifas->tarifa_hora_adicional;
    if (!isfinite(registro.valor_pago)) {
        if (lista_inserir(lista, &removido) != 1) {
            fprintf(stderr, "ERRO: saida sem valor calculavel e veiculo nao restaurado.\n");
        }
        puts("Valor calculado fora do intervalo permitido; checkout cancelado.");
        return;
    }
    if (!pilha_empilhar(historico, &registro)) {
        if (lista_inserir(lista, &removido) != 1) {
            fprintf(stderr, "ERRO: saida sem registro e veiculo nao restaurado.\n");
        }
        puts("Nao foi possivel registrar o checkout; saida cancelada.");
        return;
    }

    printf("Checkout concluido. Permanencia: %d min; valor: R$ %.2f\n",
           registro.minutos_permanencia, registro.valor_pago);
    if (fila->inicio != NULL) {
        Veiculo proximo = fila->inicio->veiculo;
        if (lista_inserir(lista, &proximo) == 1) {
            if (fila_desenfileirar(fila, NULL)) {
                printf("Veiculo %s movido da fila para o patio.\n", proximo.placa);
            }
        } else {
            fprintf(stderr, "Falha ao mover o primeiro veiculo da fila para o patio.\n");
        }
    }
}

static void listar_patio(const ListaEstacionamento *lista)
{
    const NoEstacionamento *atual;
    char hora[6];

    if (lista->inicio == NULL) {
        puts("Nao ha veiculos estacionados.");
        return;
    }
    printf("\nPatio: %lu/%lu vagas ocupadas\n",
           (unsigned long)lista->qtde,
           (unsigned long)lista->capacidade_maxima);
    for (atual = lista->inicio; atual != NULL; atual = atual->proximo) {
        veiculo_formatar_hora(atual->veiculo.hora_entrada, hora, sizeof(hora));
        printf("%-9s | %-29s | %-19s | entrada %s | %s\n",
               atual->veiculo.placa, atual->veiculo.modelo,
               atual->veiculo.cor, hora,
               atual->veiculo.tipo == 1 ? "Carro" : "Moto");
    }
}

static void consultar_veiculo(ListaEstacionamento *lista, FilaEspera *fila)
{
    char placa[TAM_PLACA];
    Veiculo *veiculo;
    int resposta;

    resposta = ler_placa(placa);
    if (resposta == 0) {
        return;
    }
    if (resposta < 0) {
        puts("Placa invalida.");
        return;
    }
    veiculo = lista_buscar(lista, placa);
    if (veiculo != NULL) {
        printf("Veiculo %s esta no patio (entrada %02d:%02d).\n",
               veiculo->placa, veiculo->hora_entrada / 60,
               veiculo->hora_entrada % 60);
        return;
    }
    veiculo = fila_buscar(fila, placa);
    if (veiculo != NULL) {
        puts("Veiculo esta na fila de espera.");
    } else {
        puts("Veiculo nao encontrado.");
    }
}

static void listar_fila(const FilaEspera *fila)
{
    const NoFila *atual;
    size_t posicao = 1;

    if (fila->inicio == NULL) {
        puts("Fila de espera vazia.");
        return;
    }
    puts("Fila de espera (ordem de chegada):");
    for (atual = fila->inicio; atual != NULL; atual = atual->proximo) {
        printf("%lu. %s | %s | %s\n", (unsigned long)posicao++,
               atual->veiculo.placa, atual->veiculo.modelo, atual->veiculo.cor);
    }
}

static void relatorio_historico(const PilhaHistorico *historico)
{
    const NoHistorico *atual;
    double total = 0.0;
    size_t posicao = 1;
    char entrada[6];
    char saida[6];

    if (historico->topo == NULL) {
        puts("Ainda nao ha atendimentos finalizados.");
        return;
    }
    puts("Historico (mais recente primeiro):");
    for (atual = historico->topo; atual != NULL; atual = atual->proximo) {
        veiculo_formatar_hora(atual->registro.veiculo.hora_entrada,
                              entrada, sizeof(entrada));
        veiculo_formatar_hora(atual->registro.hora_saida,
                              saida, sizeof(saida));
        printf("%lu. %s | %s | %s-%s | %d min | R$ %.2f\n",
               (unsigned long)posicao++, atual->registro.veiculo.placa,
               atual->registro.veiculo.modelo, entrada, saida,
               atual->registro.minutos_permanencia,
               atual->registro.valor_pago);
        total += atual->registro.valor_pago;
    }
    printf("Total arrecadado registrado: R$ %.2f\n", total);
}

static void configurar(ListaEstacionamento *lista, FilaEspera *fila,
                       Tarifas *tarifas)
{
    int capacidade;
    int resposta;
    double primeira_hora;
    double adicional;

    resposta = ler_inteiro("Nova capacidade (maior que zero): ", &capacidade);
    if (resposta == 0) {
        return;
    }
    if (resposta < 0 || capacidade <= 0 ||
        (size_t)capacidade < lista->qtde) {
        puts("Capacidade invalida ou menor que a ocupacao atual.");
        return;
    }
    resposta = ler_decimal("Tarifa da primeira hora (R$): ", &primeira_hora);
    if (resposta == 0) {
        return;
    }
    if (resposta < 0) {
        puts("Tarifa invalida.");
        return;
    }
    resposta = ler_decimal("Tarifa por hora adicional (R$): ", &adicional);
    if (resposta == 0) {
        return;
    }
    if (resposta < 0) {
        puts("Tarifa invalida.");
        return;
    }
    lista->capacidade_maxima = (size_t)capacidade;
    tarifas->tarifa_primeira_hora = primeira_hora;
    tarifas->tarifa_hora_adicional = adicional;

    while (lista->qtde < lista->capacidade_maxima && fila->inicio != NULL) {
        Veiculo proximo = fila->inicio->veiculo;
        if (lista_inserir(lista, &proximo) != 1) {
            break;
        }
        fila_desenfileirar(fila, NULL);
        printf("Veiculo %s movido da fila para o patio.\n", proximo.placa);
    }
    puts("Capacidade e tarifas atualizadas.");
}

static void exibir_menu(void)
{
    puts("\n=== Controle de Estacionamento ===");
    puts("1. Registrar entrada");
    puts("2. Registrar saida / checkout");
    puts("3. Listar veiculos estacionados");
    puts("4. Consultar veiculo por placa");
    puts("5. Visualizar fila de espera");
    puts("6. Relatorio de historico e faturamento");
    puts("7. Configurar capacidade e tarifas");
    puts("0. Salvar e sair");
}

static void carregar_estado(ListaEstacionamento *lista, FilaEspera *fila,
                            PilhaHistorico *historico, Tarifas *tarifas)
{
    int resultado;

    resultado = persistencia_carregar_patio(lista, tarifas, ARQUIVO_PATIO);
    if (resultado < 0) {
        fprintf(stderr, "Aviso: nao foi possivel carregar %s; usando patio vazio.\n",
                ARQUIVO_PATIO);
        lista_limpar(lista);
        lista->capacidade_maxima = 10;
    } else if (resultado == 0) {
        puts("Arquivo de patio inexistente; iniciando com 10 vagas.");
    }
    resultado = persistencia_carregar_fila(fila, ARQUIVO_FILA);
    if (resultado < 0) {
        fprintf(stderr, "Aviso: nao foi possivel carregar %s; fila iniciada vazia.\n",
                ARQUIVO_FILA);
        fila_limpar(fila);
    }
    resultado = persistencia_carregar_historico(historico, ARQUIVO_HISTORICO);
    if (resultado < 0) {
        fprintf(stderr, "Aviso: nao foi possivel carregar %s; historico iniciado vazio.\n",
                ARQUIVO_HISTORICO);
        pilha_limpar(historico);
    }
}

int main(void)
{
    ListaEstacionamento lista;
    FilaEspera fila;
    PilhaHistorico historico;
    Tarifas tarifas;
    int opcao;
    int leitura;
    int executando = 1;

    lista_inicializar(&lista, 10);
    fila_inicializar(&fila);
    pilha_inicializar(&historico);
    tarifas.tarifa_primeira_hora = 10.0;
    tarifas.tarifa_hora_adicional = 5.0;
    carregar_estado(&lista, &fila, &historico, &tarifas);

    while (executando) {
        exibir_menu();
        leitura = ler_inteiro("Opcao: ", &opcao);
        if (leitura == 0) {
            break;
        }
        if (leitura < 0) {
            puts("Digite uma opcao numerica valida.");
            continue;
        }
        switch (opcao) {
        case 1:
            registrar_entrada(&lista, &fila);
            if (!salvar_estado(&lista, &fila, &historico, &tarifas)) {
                fprintf(stderr, "Aviso: o estado atual pode nao estar persistido.\n");
            }
            break;
        case 2:
            registrar_saida(&lista, &fila, &historico, &tarifas);
            if (!salvar_estado(&lista, &fila, &historico, &tarifas)) {
                fprintf(stderr, "Aviso: o estado atual pode nao estar persistido.\n");
            }
            break;
        case 3:
            listar_patio(&lista);
            break;
        case 4:
            consultar_veiculo(&lista, &fila);
            break;
        case 5:
            listar_fila(&fila);
            break;
        case 6:
            relatorio_historico(&historico);
            break;
        case 7:
            configurar(&lista, &fila, &tarifas);
            if (!salvar_estado(&lista, &fila, &historico, &tarifas)) {
                fprintf(stderr, "Aviso: o estado atual pode nao estar persistido.\n");
            }
            break;
        case 0:
            executando = 0;
            break;
        default:
            puts("Opcao inexistente.");
        }
    }

    if (!salvar_estado(&lista, &fila, &historico, &tarifas)) {
        fprintf(stderr, "Falha ao persistir completamente o estado ao encerrar.\n");
    }
    lista_limpar(&lista);
    fila_limpar(&fila);
    pilha_limpar(&historico);
    puts("Estado salvo. Sistema encerrado.");
    return 0;
}
