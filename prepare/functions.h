#ifndef FUNCTIONS_H
#define FUNCTIONS_H

typedef struct task {
    int id;           // identificador da tarefa 
    int duration;     // duração em segundos 
} Task;

// Protótipo da função para guardar a tarefa
void save_task_binary(Task t);

#endif // FUNCTIONS_H