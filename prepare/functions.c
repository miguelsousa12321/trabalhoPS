#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>      // <- para errno e EEXIST
#include "functions.h"

void save_task_binary(Task t) {
    char nome_ficheiro[100];

    // Cria a pasta "tasks_data" no diretório superior se não existir
    if (mkdir("../tasks_data", 0700) == -1 && errno != EEXIST) /* ignora erro se o motivo for que o diretótio já existir */{
        perror("Erro a criar diretório ../tasks_data");
        return;
    }

    // Nome do ficheiro inclui o ID da tarefa
    snprintf(nome_ficheiro, sizeof(nome_ficheiro), "../tasks_data/task_%d.bin", t.id); 

    // Abre para escrita binária: cria se não existir, substitui se já existir
    int fd = open(nome_ficheiro, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Erro ao criar/abrir ficheiro da tarefa");
        return;
    }

    // Escreve a struct completa no ficheiro
    ssize_t bytes = write(fd, &t, sizeof(Task));
    if (bytes != (ssize_t)sizeof(Task)) {
        perror("Erro ao escrever dados binários");
        close(fd); // tenta fechar mesmo em erro
        return;
    }

    if (close(fd) == -1) {
        perror("Erro ao fechar ficheiro da tarefa");
        return;
    }

    printf("Tarefa %d (Duração: %ds) guardada com sucesso em %s.\n",
           t.id, t.duration, nome_ficheiro);
}
