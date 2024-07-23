/////////////////////////////////////////////////////////////////////////////////////////////////////////
//TRABALHO ESCALONADOR   - SISTEMAS OPERACIONAIS
//MATHEUS LOPES DE SOUZA - 190043831
// VERSÃO SISTEMA OPERACIONAL -> Ubuntu 22.04.3 LTS
// VERSÃO COMPILADOR          -> gcc 11.4.0
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define LEN_COMMAND 7
#define LEN_LINE 30

int high_priority_queue_id;

int amount_lines(){
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

int verificaCores(int cores){
     if(cores >= 1){
        return cores--;
     }
     return 0;
}

struct messages {
	long mtype;
	char mtext[100];
} msg_send, msg_rcvd;


int main(int argc, char *argv[]){
    //struct f_buff proc[8];
    //struct f_buff queue_ready[8];
    int n_cores, proc_name;
    int amount_process = amount_lines();
    char command[8], proc_dependency[15];
    

     //verify command line input
    n_cores = argc == 2 ? atoi(argv[1]) : 0;
    if(n_cores == 0){fprintf(stderr, "Insuficient arguments\n"); exit(1);}

    //process struct
    typedef struct {
        int pid;
        int executed;

        int name;
        char command[8];
        int input_dependency[15];
    } f_buff;


    f_buff processos[amount_process];

    FILE *fp;
    if((fp = fopen("input.txt", "r")) < 0){ //Leitura de dados do arquivo
        fprintf(stderr, "Erro ao abrir arquivo\n");
        exit(2);
    }

    //Atribui valores para struct processos
    int i=0, j=0, queue_size=0;
    int pid;
    char *token;
    int first_process;
    while((fscanf(fp,"%d %s %s\n", &proc_name, command, proc_dependency)) != EOF){
        processos[i].name = proc_name;
        strcpy(processos[i].command, command);
        

        if(strcmp(proc_dependency, "0,#") == 0){
            first_process = processos[i].name;
        }

        token = strtok(proc_dependency, " ,#");
        j=0;
        while (token != NULL){
            processos[i].input_dependency[j] = atoi(token);
            token = strtok(NULL, " ,#");
            j++;
        }
        i++;
    }  

    fclose(fp); // Fechar o arquivo após o uso
        //creating process
    int fd[2], status;
    char msg[30];
    char readbuffer[30];
    
    //Cria fila de mensagens
    int queue_id;
    if ((queue_id = msgget(0x8551, IPC_CREAT | 0x1FF)) < 0) {
		printf("Error creating queue\n");
		exit(5);
	}


    for(int i=0; i<amount_process; i++){
        //verifica se pipe deu certo
        if(pipe(fd) < 0){
            printf("Erro no pipe");
        }
        
        if((pid = fork()) < 0)
        {
            printf("erro no fork");
            exit(1);
        }

        if(pid == 0)
        {
            /* Child process closes up input side of pipe */
            close(fd[0]);

            /* Send "msg" through the output side of pipe */
            if(processos[i].name == first_process){
                strcpy(msg, "first");
                if(write(fd[1], msg, 30) < 0){
                    printf("Erro write");
                }
            }else{
                strcpy(msg, "n");
                if(write(fd[1], msg, 30) < 0){
                    printf("Erro write");
                }
            }
            sleep(1);

            if (execl(processos[i].command, processos[i].command, (char *) 0) < 0) {
			    printf("Error in execl = %d\n", errno);
			    exit(3);
		    }
            
            if ((msgrcv(queue_id, &msg_rcvd, sizeof(msg_rcvd), 0, IPC_NOWAIT)) >= 0) {
                printf("MENSAGEM RECEBIDA!");
            }

            //verificar se dependencia ja foi executada, nao ha processos executando e tem core disponivel
            //pegar pid e sinalizar que tal processo terminou
            
            exit(0);
        }
        
        /* Parent process closes up output side of pipe */
        close(fd[1]);

        /* Read in a string from the pipe */
        if(read(fd[0], msg, sizeof(msg)) < 0){
            printf("Erro read");
        }

        if(strcmp(msg, "first") == 0) printf("estou com primeiro\n");
        if(strcmp(msg, "first") != 0) kill(pid, SIGSTOP);
        wait(&status);
    }   
    return 0;
}
