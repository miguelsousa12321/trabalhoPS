#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "functions.h"

int ler_tarefa_binario(int id, Task *t) {
    char nome_ficheiro[256];
    int n = snprintf(nome_ficheiro, sizeof(nome_ficheiro), "../tasks_data/task_%d.bin", id);
    
    if (n < 0 || n >= (int)sizeof(nome_ficheiro)) {
        fprintf(stderr, "Erro: Caminho do ficheiro demasiado longo.\n");
        return -1;
    }

    int fd = open(nome_ficheiro, O_RDONLY);
    if (fd < 0) {
        perror("Erro ao abrir ficheiro da tarefa");
        return -1;
    }

    ssize_t bytes = read(fd, t, sizeof(Task));
    
    if (bytes < 0) {
        perror("Erro na leitura (read)");
        close(fd);
        return -1;
    }
    
    if (bytes != (ssize_t)sizeof(Task)) {
        fprintf(stderr, "Erro: Leitura incompleta no ficheiro da tarefa %d\n", id);
        close(fd);
        return -1;
    }

    // Reporta erro no fecho mas permite que o programa continue, pois os dados já foram lidos
    if (close(fd) < 0) {
        perror("Aviso: Erro ao fechar descritor de ficheiro");
    }

    if (t->id <= 0 || t->duration <= 0) {
        fprintf(stderr, "Erro: Dados inválidos lidos (Ficheiro: %d | ID: %d | Dur: %d)\n", 
                id, t->id, t->duration);
        return -1;
    }

    return 0;
}

int comparar_tarefas(const void *a, const void *b) {
    const Task *t1 = (const Task *)a;
    const Task *t2 = (const Task *)b;
    if (t1->duration != t2->duration) return t1->duration - t2->duration;
    return t1->id - t2->id;
}

void ordenar_sjf(Task tasks[], int n) {
    qsort(tasks, (size_t)n, sizeof(Task), comparar_tarefas);
}

void executar_fcfs(Task tasks[], int n, double *turnaround_medio) {
    double total_turnaround = 0.0;
    time_t inicio_global;
    time(&inicio_global); 

    for (int i = 0; i < n; i++) {
        printf("A executar tarefa %d (Duração: %ds)...\n", tasks[i].id, tasks[i].duration);
        sleep((unsigned int)tasks[i].duration);

        time_t fim;
        time(&fim);
        total_turnaround += difftime(fim, inicio_global);
    }
    *turnaround_medio = total_turnaround / (double)n;
}

void executar_paralelo(Task tasks[], int n, int N, double *turnaround_medio) {
    if (N <= 0) {
        fprintf(stderr, "Erro crítico: N deve ser >= 1\n");
        exit(1);
    }

    time_t inicio_global;
    time(&inicio_global);

    int ativos = 0;
    double total_turnaround = 0.0;

    for (int i = 0; i < n; i++) {
        if (ativos >= N) {
            int status;
            if (waitpid(-1, &status, 0) < 0) { 
                perror("Erro no waitpid");
                exit(1);
            }
            time_t agora;
            time(&agora);
            total_turnaround += difftime(agora, inicio_global);
            ativos--;
        }

        pid_t pid = fork(); 
        if (pid < 0) {
            perror("Erro no fork");
            exit(1);
        }

        if (pid == 0) {
            printf("[FILHO %d] Tarefa %d em curso...\n", getpid(), tasks[i].id);
            sleep((unsigned int)tasks[i].duration);
            _exit(0);
        }
        ativos++;
    }

    while (ativos > 0) {
        int status;
        if (waitpid(-1, &status, 0) < 0) { 
            perror("Erro no waitpid final");
            exit(1);
        }
        time_t agora;
        time(&agora);
        total_turnaround += difftime(agora, inicio_global);
        ativos--;
    }
    *turnaround_medio = total_turnaround / (double)n;
}

void escrever_estatisticas(int total_tarefas, double turnaround_medio, int modo) {
    int fd = open("Estatisticas_Globais.txt", O_WRONLY | O_CREAT | O_APPEND, 0644); 
    if (fd < 0) {
        perror("Erro ao abrir ficheiro de estatísticas");
        return;
    }

    const char *nomes_modos[] = {"FCFS seq", "FCFS par", "SJF seq", "SJF par"};
    const char *modo_str = (modo >= 0 && modo <= 3) ? nomes_modos[modo] : "Desconhecido";

    dprintf(fd, "===== MODO: %s =====\n", modo_str);
    dprintf(fd, "Total Tarefas Executadas: %d\n", total_tarefas);
    dprintf(fd, "Turnaround Time Médio: %.2f segundos\n\n", turnaround_medio); 

    if (close(fd) < 0) perror("Aviso: Erro ao fechar ficheiro de estatísticas");
}