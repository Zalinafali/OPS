#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     exit(EXIT_FAILURE))


void create_children(int n, int k) {
    printf("%d Start: %d\n",getpid(), ++k);
	switch (fork()) {
		case 0:
            if(k < n){
                create_children(n,k);
                wait(NULL);
            }
            else sleep(1);
            printf("%d End: %d\n",getpid(), k);
            exit(EXIT_SUCCESS);

		case -1: ERR("Fork:");
	}
}

void usage(char * name){
	fprintf(stderr,"USAGE: %s n\n",name);
	fprintf(stderr,"0<n<=10 - number of children\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int n, fd1[2];
    int k=0;
	if(2!=argc) usage(argv[0]);
	n = atoi(argv[1]);
	if (n<=0||n>20) usage(argv[0]);

	create_children(n,k);
    wait(NULL);

	return EXIT_SUCCESS;
}