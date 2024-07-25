/////////////////////////////////////////////////////////////////////////////////////////////////////////
//TRABALHO ESCALONADOR   - SISTEMAS OPERACIONAIS
//MATHEUS LOPES DE SOUZA - 190043831
//PEDRO HENRIQUE DE MOURA - 190018810
// VERSÃO SISTEMA OPERACIONAL -> Ubuntu 22.04.3 LTS
// VERSÃO COMPILADOR          -> gcc 11.4.0
// 
// A estrategia utilizada foi de First Come First Served com base nas dependencias dos processos.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>

//process struct
typedef struct Process {
    int pid;
    bool executed;

    int name;
    char command[8];
    int input_dependency[15];
    int num_dependencies;
    int pipe_fd[2];
    double start_time;
    double exec_time;
} Process;

int count_lines(){
    FILE *file;
    char line[30];
    int count=0;
    if((file = fopen("input.txt", "r")) == NULL){ //arquivo
        fprintf(stderr, "Erro ao abrir arquivo\n");
        exit(2);
    }

    while(fgets(line, sizeof(line), file)){
        count++;
    }

    fclose(file);

    return count;
}

int main(int argc, char *argv[]) {
    struct timespec start, end;
    int number_processes = count_lines();
    Process processes[number_processes];
    int n_cores, proc_name;
    char command[8], proc_dependency[15];

    //Verifica se foi informado numero de cores input
    n_cores = argc == 2 ? atoi(argv[1]) : 0;
    if(n_cores == 0){fprintf(stderr, "Insuficient arguments\n"); exit(1);}
 
// **** SECAO DE LER INPUT E ARMAZENAR EM STRUCT **** ////
    FILE *fp;
    if((fp = fopen("input.txt", "r")) < 0){ //Leitura de dados do arquivo
        fprintf(stderr, "Erro ao abrir arquivo\n");
        exit(2);
    }

    int i=0, j=0;;
    int pid;
    char *token;
    int first_process;
    while((fscanf(fp,"%d %s %s\n", &proc_name, command, proc_dependency)) != EOF){
        processes[i].pid = 0;
        processes[i].executed = false;
        processes[i].name = proc_name;
        processes[i].num_dependencies = 0;
        processes[i].exec_time = 0.0;
        strcpy(processes[i].command, command);
        pipe(processes[i].pipe_fd);
        
        if(strcmp(proc_dependency, "0,#") == 0){
            first_process = processes[i].name;
        }

        token = strtok(proc_dependency, " ,#");
        j=0;
        while (token != NULL){
            processes[i].input_dependency[j] = atoi(token);
            processes[i].num_dependencies++;
            token = strtok(NULL, " ,#");
            j++;
        }
        i++;
    }  

    fclose(fp); // Fechar o arquivo após o uso

//*****************************//

// ********* SECAO DE ESCALONADOR ******** ////

    int status;
    int active_processes = 0;
    int executed_processes = 0;
    while(executed_processes < number_processes){
        for (int k = 0; k < number_processes; k++) {
            if(processes[k].pid == 0 && processes[k].executed == false){
                // Verifica dependencias
                int can_run = 1;
                for (int l = 0; l < processes[k].num_dependencies; l++) {
                    int dep_id = processes[k].input_dependency[l] - 1;
                    if(dep_id < 0 && processes[k].executed == false){
                        clock_gettime(CLOCK_MONOTONIC, &start);
                        break;
                    }
                    else if (processes[dep_id].executed == false) {
                        can_run = 0;
                        break;
                    }
                }
                //***********/

                if (can_run && active_processes < n_cores) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        // Código do processo filho
                        close(processes[k].pipe_fd[0]);
                        if (execl(processes[k].command, processes[k].command, (char *) 0) < 0) {
			                exit(3);
		                }
                        exit(0);
                    } else {
                        printf("Processo %d executando\n", processes[k].name);
                        struct timespec start_time;
                        clock_gettime(CLOCK_MONOTONIC, &start_time);
                        processes[k].start_time = (start_time.tv_sec - start.tv_sec) + (start_time.tv_nsec - start.tv_nsec) / 1e9;
                        processes[k].pid = pid;
                        active_processes++;
                    }
                }
            }
        }
        for (int k = 0; k < number_processes; k++) {
            if (processes[k].pid > 0) {
                // printf("Processo %d executando \n", processes[k].name);

                if(active_processes == n_cores){
                    // printf("Todos os cores ocupados\n");
                    int status;
                    waitpid(pid, &status, 0);

                    // printf("Processo %d finalizado\n", processes[k].name);
                    processes[k].executed = true;
                    executed_processes++;
                    active_processes--;
                    processes[k].pid = 0;
                    struct timespec final_time;
                    clock_gettime(CLOCK_MONOTONIC, &final_time);
                    processes[k].exec_time = (final_time.tv_sec - start.tv_sec) + (final_time.tv_nsec - start.tv_nsec) / 1e9; 
                    processes[k].exec_time = processes[k].exec_time - processes[k].start_time;

                    close(processes[k].pipe_fd[1]);
                }

                else if (waitpid(processes[k].pid, &status, WNOHANG) == processes[k].pid){
                    // printf("Processo %d finalizado\n", processes[k].name);
                    processes[k].executed = true;
                    active_processes--;
                    executed_processes++;
                    processes[k].pid = 0;
                    struct timespec final_time;
                    clock_gettime(CLOCK_MONOTONIC, &final_time);
                    processes[k].exec_time = (final_time.tv_sec - start.tv_sec) + (final_time.tv_nsec - start.tv_nsec) / 1e9;
                    processes[k].exec_time = processes[k].exec_time - processes[k].start_time;

                    close(processes[k].pipe_fd[1]);
                }
                
                // int status;
                // waitpid(pid, &status, 0);

                // printf("Scheduler: Processo %d finalizado\n", processes[k].name);
                // processes[k].executed = true;
                // executed_processes++;
                // processes[k].pid = 0;

                // close(processes[k].pipe_fd[1]);

            }    
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    // Calcula o tempo total de execução
    double makespan = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    for (int k = 0; k < number_processes; k++) {
        printf("Tempo de execucao Processo %d: %.2f segundos\n", processes[k].name, processes[k].exec_time);
        close(processes[k].pipe_fd[0]);
        close(processes[k].pipe_fd[1]);
    }

    printf("Tempo makespan total: %.2f segundos\n", makespan);
}