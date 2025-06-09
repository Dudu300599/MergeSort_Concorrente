#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"

#define MAX_LINHA 100
#define MAX_REGISTROS 500000

typedef struct {
    float altura;
    float peso;
    int idade;
} Pessoa;

typedef struct {
    Pessoa *arr;
    int l;
    int r;
} ThreadArgs;

Pessoa pessoas[MAX_REGISTROS];
int threads_disponiveis;
pthread_mutex_t mutex;

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

void *mergeSortConcurrent(void *args);

void mergeSort(Pessoa arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        pthread_t thread1, thread2;
        int criar_thread1 = 0, criar_thread2 = 0;

        // Verifica se podemos criar thread para esquerda
        pthread_mutex_lock(&mutex);
        if (threads_disponiveis > 0) {
            threads_disponiveis--;
            criar_thread1 = 1;
        }
        pthread_mutex_unlock(&mutex);

        if (criar_thread1) {
            ThreadArgs *args1 = malloc(sizeof(ThreadArgs));
            args1->arr = arr;
            args1->l = l;
            args1->r = m;
            pthread_create(&thread1, NULL, mergeSortConcurrent, args1);
        } else {
            mergeSort(arr, l, m);
        }

        // Verifica se podemos criar thread para direita
        pthread_mutex_lock(&mutex);
        if (threads_disponiveis > 0) {
            threads_disponiveis--;
            criar_thread2 = 1;
        }
        pthread_mutex_unlock(&mutex);

        if (criar_thread2) {
            ThreadArgs *args2 = malloc(sizeof(ThreadArgs));
            args2->arr = arr;
            args2->l = m + 1;
            args2->r = r;
            pthread_create(&thread2, NULL, mergeSortConcurrent, args2);
        } else {
            mergeSort(arr, m + 1, r);
        }

        // Espera as threads terminarem
        if (criar_thread1) {
            pthread_join(thread1, NULL);
            pthread_mutex_lock(&mutex);
            threads_disponiveis++;
            pthread_mutex_unlock(&mutex);
        }

        if (criar_thread2) {
            pthread_join(thread2, NULL);
            pthread_mutex_lock(&mutex);
            threads_disponiveis++;
            pthread_mutex_unlock(&mutex);
        }

        // Merge final (sequencial)
        merge(arr, l, m, r);
    }
}

void *mergeSortConcurrent(void *args) {
    ThreadArgs *targs = (ThreadArgs *)args;
    mergeSort(targs->arr, targs->l, targs->r);
    free(targs);
    return NULL;
}

int ler_csv(const char *nome_arquivo, Pessoa pessoas[]) {
    FILE *fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror("Erro ao abrir o CSV");
        exit(EXIT_FAILURE);
    }

    char linha[MAX_LINHA];
    int count = 0;

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
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada.csv> <num_threads>\n", argv[0]);
        return 1;
    }

    double start, finish, elapsed;

    const char *arquivo_entrada = argv[1];
    int num_threads = atoi(argv[2]);

    // Configura threads_disponiveis com o valor do usuário
    threads_disponiveis = num_threads;

    pthread_mutex_init(&mutex, NULL);

    int total = ler_csv(arquivo_entrada, pessoas);

    GET_TIME(start);

    mergeSort(pessoas, 0, total - 1);

    GET_TIME(finish);
    elapsed = finish - start;

    escrever_csv("dados_pessoas_ordenado.csv", pessoas, total);

    printf("Ordenação concorrente concluída. CSV salvo em: dados_pessoas_ordenado.csv\n");
    printf("Tempo de Ordenação: %f segundos\n", elapsed);

    pthread_mutex_destroy(&mutex);

    return 0;
}
