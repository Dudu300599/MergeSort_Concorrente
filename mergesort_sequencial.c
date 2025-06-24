#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timer.h"

#define MAX_LINHA 100
#define MAX_REGISTROS 1000000

typedef struct {
    float altura;
    float peso;
    int idade;
} Pessoa;

void merge(Pessoa arr[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    Pessoa *L = malloc(n1 * sizeof(Pessoa));
    Pessoa *R = malloc(n2 * sizeof(Pessoa));

    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i].idade < R[j].idade ||
           (L[i].idade == R[j].idade && L[i].peso < R[j].peso) ||
           (L[i].idade == R[j].idade && L[i].peso == R[j].peso && L[i].altura < R[j].altura)) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void mergeSort(Pessoa arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l)/2;
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
        merge(arr, l, m, r);
    }
}

int ler_csv(const char *nome_arquivo, Pessoa pessoas[]) {
    FILE *fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror("Erro ao abrir o CSV");
        exit(EXIT_FAILURE);
    }

    char linha[MAX_LINHA];
    int count = 0;

    // Ignorar cabeçalho
    fgets(linha, MAX_LINHA, fp);

    while (fgets(linha, MAX_LINHA, fp) && count < MAX_REGISTROS) {
        sscanf(linha, "%f,%f,%d", &pessoas[count].altura, &pessoas[count].peso, &pessoas[count].idade);
        count++;
    }

    fclose(fp);
    return count;
}

void escrever_csv(const char *nome_arquivo, Pessoa pessoas[], int n) {
    FILE *fp = fopen(nome_arquivo, "w");
    if (!fp) {
        perror("Erro ao criar CSV ordenado");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "altura,peso,idade\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%.2f,%.1f,%d\n", pessoas[i].altura, pessoas[i].peso, pessoas[i].idade);
    }

    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_entrada.csv>\n", argv[0]);
        return 1;
    }

    double start, finish, elapsed;

    const char *arquivo_entrada = argv[1];
    const char *arquivo_saida = "dados_pessoas_ordenado.csv";
    Pessoa pessoas[MAX_REGISTROS];

    int total = ler_csv(arquivo_entrada, pessoas);

    GET_TIME(start);
    mergeSort(pessoas, 0, total - 1);
    GET_TIME(finish);
    elapsed = finish - start;
    escrever_csv(arquivo_saida, pessoas, total);

    printf("Ordenação concluída. CSV salvo em: %s\n", arquivo_saida);
    printf("Tempo de Ordenação: %f segundos\n", elapsed);
    return 0;
}
