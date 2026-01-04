
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define MIN_DUR 1
#define MAX_DUR 5

int main(int argc, char *argv[]) {
    // Verifica se o número correto de argumentos foi fornecido
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <task_id> <duration>\n", argv[0]);
        return 1;
    }

    // Preenche a estrutura com os dados recebidos 
    Task t;
    t.id = atoi(argv[1]);
    t.duration = atoi(argv[2]);

    // Validações mínimas (com atoi)
    if (t.id <= 0) {
        fprintf(stderr, "Erro: task_id tem de ser > 0.\n");
        return 1;
    }

    if (t.duration < MIN_DUR || t.duration > MAX_DUR) {
        fprintf(stderr, "Erro: duration tem de estar entre %d e %d.\n", MIN_DUR, MAX_DUR);
        return 1;
    }
    // Executa a função de gravação modular
    save_task_binary(t);
    return 0;
}
