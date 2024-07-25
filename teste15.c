#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main(int argc, char **argv){	
    long i;

    for(i=0; i<8000000000; i++);
	//comment

    return 0;
}