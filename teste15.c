#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct message {
	long mtype;
	char mtext[100];
};

int main(int argc, char **argv){
	struct message msg_send;
    long i;
	pid_t pid;
    int queue_id;
	
    msg_send.mtype = getpid();

    strcpy(msg_send.mtext, "PROCESSO");


    if ((queue_id = msgget(0x8551, 0x1FF)) < 0) {
		printf("Error getting queue\n");
		exit(5);
	}

	printf("Processo %s executando\n", argv[1]);
	sleep(1);
    //for(i=0; i<8000000000; i++);
	//comment

    if (msgsnd(queue_id, &msg_send, sizeof(msg_send), IPC_NOWAIT) < 0) {
		printf("Error sending message\n");
		exit(4);
	}

	printf("Processo encerrou\n");
    return 0;
}