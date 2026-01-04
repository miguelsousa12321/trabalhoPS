#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"

/* Política de escalonamento: FCFS (ordem de chegada) ou SJF (menor duração primeiro) */
typedef enum {
    FCFS = 0,
    SJF  = 1
} Policy;

/*
 * Função auxiliar para mostrar ao utilizador como executar o programa,
 * caso os argumentos sejam inválidos.
 */



/* O nosso run.sh chama:
    - ./scheduler <num_tasks>                 (modo 0: FCFS sequencial)
    - ./scheduler <num_tasks> <N> fcfs        (modo 1: FCFS paralelo se N>1)
    - ./scheduler <num_tasks> 1 sjf           (modo 2: SJF sequencial se N=1)
    - ./scheduler <num_tasks> <N> sjf         (modo 3: SJF paralelo se N>1)
*/

void usage(const char *prog) {
    fprintf(stderr,
        "Uso do scheduler:\n\n"
        "  %s <num_tasks>\n"
        "      -> Sem Otimizacoes (FCFS sequencial)\n\n"
        "  %s <num_tasks> <N> fcfs\n"
        "      -> Otimizacao do Paralelismo (FCFS paralelo, N processos)\n\n"
        "  %s <num_tasks> 1 sjf\n"
        "      -> Otimizacao do SJF (SJF sequencial)\n\n"
        "  %s <num_tasks> <N> sjf\n"
        "      -> Todas as Otimizacoes (SJF + paralelismo com N processos)\n\n"
        "Onde:\n"
        "  num_tasks : numero de tarefas a executar\n"
        "  N         : numero maximo de processos em paralelo (>=1)\n",
        prog, prog, prog, prog
    );
}



int main(int argc, char *argv[]) {


    if (argc != 2 && argc != 4) {
        usage(argv[0]);
        return 1;
    }

    int num_tasks = atoi(argv[1]);
    if (num_tasks <= 0) {
        fprintf(stderr, "Erro: num_tasks tem de ser > 0\n");
        return 1;
    }

    /* Defaults para o caso argc==2:
       - N=1 => execução sequencial
       - policy=FCFS
    */

    int N = 1;
    Policy policy = FCFS;

    if (argc == 4) {
        N = atoi(argv[2]);
        if (N <= 0) {
            fprintf(stderr, "Erro: N tem de ser >= 1\n");
            return 1;
        }

        if (strcmp(argv[3], "fcfs") == 0) /*strcmp - compara duas strings e devolve zero quando são iguais*/ {
            policy = FCFS;
        } else if (strcmp(argv[3], "sjf") == 0) {
            policy = SJF;
        } else {
            fprintf(stderr, "Erro: politica invalida '%s' (use fcfs ou sjf)\n", argv[3]);
            return 1;
        }
    }

    /* Carregar tarefas (binários produzidos pelo prepare) */
    Task tasks[num_tasks];
    for (int i = 0; i < num_tasks; i++) {
        if (ler_tarefa_binario(i + 1, &tasks[i]) < 0) {
            fprintf(stderr, "Erro ao ler tarefa %d\n", i + 1);
            return 1;
        }
    }

    /* Aplicar política:
        - FCFS: não altera a ordem
        - SJF : ordena por duração (e desempata por id)
    */

    if (policy == SJF) {
        ordenar_sjf(tasks, num_tasks);
    }

    /* Executar e calcular turnaround médio */
    double turnaround_medio = 0.0;

    if (N == 1) {
        executar_fcfs(tasks, num_tasks, &turnaround_medio);
    } else {
        executar_paralelo(tasks, num_tasks, N, &turnaround_medio);
    }

    /* Inferir modo (apenas para escrever o cabeçalho certo no ficheiro de estatísticas) */
    int modo = 0;
    if (policy == FCFS && N == 1) modo = 0;
    if (policy == FCFS && N > 1)  modo = 1;
    if (policy == SJF  && N == 1) modo = 2;
    if (policy == SJF  && N > 1)  modo = 3;

    /* Escrever estatísticas globais (APPEND em blocos) */
    escrever_estatisticas(num_tasks, turnaround_medio, modo);

    return 0;
}