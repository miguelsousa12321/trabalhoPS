#ifndef FUNCTIONS_H 
#define FUNCTIONS_H

typedef struct task {
    int id;
    int duration; // duração em segundos
} Task;

/* Lê a tarefa binária ../tasks_data/task_<id>.bin para *t. Retorna 0 em sucesso. */
int ler_tarefa_binario(int id, Task *t);

/* Ordena por SJF (duração crescente). Desempate por id. */
void ordenar_sjf(Task tasks[], int n);

/* Execução sequencial (FCFS na ordem do array) */
void executar_fcfs(Task tasks[], int n, double *turnaround_medio);

/* Execução paralela com limite N (fork + waitpid). */
void executar_paralelo(Task tasks[], int n, int N, double *turnaround_medio);

/* Escreve estatísticas em APPEND em blocos  */
void escrever_estatisticas(int total_tarefas, double turnaround_medio, int modo);

#endif
