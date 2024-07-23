#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>

struct message {
	long mtype;
	char mtext[100];
} msg_send;

int main(){
    long i;
    int queue_id;
    msg_send.mtype = 1;
    strcpy(msg_send.mtext, "PROCESSO");


    if ((queue_id = msgget(0x8551, 0x1FF)) < 0) {
		printf("Error getting queue\n");
		exit(5);
	}

    for(i=0; i<8000000000; i++);
    if (msgsnd(queue_id, &msg_send, sizeof(msg_send), IPC_NOWAIT) < 0) {
		printf("Error sending message\n");
		exit(4);
	}

	printf("Processo PID = 1 encerrou\n");
    return 0;
}