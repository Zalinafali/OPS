#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <aio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#define BUFFERS 3

void error(char *);
void usage(char *);

//void readInput(int* cycles, int* Nb, int* Sb){}

void error(char *msg){
	perror(msg);
	exit(EXIT_FAILURE);
}
void usage(char *progname){
	fprintf(stderr, "%s workfile blocksize\n", progname);
	fprintf(stderr, "workfile - path to the file to work on\n");
	fprintf(stderr, "Nb - number of blocks\n");
	fprintf(stderr, "Sb - number of iterations\n");
	exit(EXIT_FAILURE);
}

void initializeBuffers(char** buffers, int Sb){
    int i;
    for (i = 0; i<BUFFERS; i++)
			if ((buffers[i] = (char *) calloc (Sb, sizeof(char))) == NULL)
				error("Cannot allocate memory");
}

void cleanUp(char** buffer){
    int i;
	for (i = 0; i<BUFFERS; i++)
		free(buffer[i]);
}

void fillaiostructs(struct aiocb *aiocbs, char **buffer, int fd, int Sb){
	int i;
	for (i = 0; i<BUFFERS; i++){
		memset(&aiocbs[i], 0, sizeof(struct aiocb));
		aiocbs[i].aio_fildes = fd;
		aiocbs[i].aio_offset = 0;
		aiocbs[i].aio_nbytes = Sb;
		aiocbs[i].aio_buf = (void *) buffer[i];
		aiocbs[i].aio_sigevent.sigev_notify = SIGEV_NONE;
	}
}

void readdata(struct aiocb *aiocbs){
	if (aio_read(aiocbs) == -1)
		error("Cannot read");
}

void writedata(struct aiocb *aiocbs){
	if (aio_write(aiocbs) == -1)
		error("Cannot write");
}

void suspend(struct aiocb *aiocbs){
	struct aiocb *aiolist[1];
	aiolist[0] = aiocbs;
	while (aio_suspend((const struct aiocb *const *) aiolist, 1, NULL) == -1){
		if (errno == EINTR) continue;
		error("Suspend error");
	}
	if (aio_error(aiocbs) != 0)
		error("Suspend error");
	if (aio_return(aiocbs) == -1)
		error("Return error");
}

void processblock(struct aiocb *aiocbs, char **buffer, int Nb, int Sb){
    int i = 0;
    readdata(&aiocbs[i]);
    suspend(&aiocbs[i]);
    printf("First block: %s\n", buffer[i]);
    shiftBuffer(buffer[i], Sb);
    printf("Changed block: %s\n", buffer[i]);
    writedata(&aiocbs[i]);
    suspend(&aiocbs[i]);
}

void shiftBuffer(char* buffer, int Sb){
    char temp1 = buffer[Sb-1], temp2;
    int i;
    for(i = Sb-1; i > 1; i--){
        temp2 = buffer[i-1];
        buffer[i-1] = temp1;
        temp1 = temp2;
    }
    temp2 = buffer[Sb-1];
    buffer[Sb-1] = temp1;
    buffer[0] = temp2;
}

int main(int argc, char *argv[]){
    char *filename, *buffer[BUFFERS];
    int fd, Nb, Sb, cycles;
    struct aiocb aiocbs[3];  
    if ((argc-2)%2 != 0)
		usage(argv[0]);
    if((Nb = atoi(argv[2])) <= 0) error("Nb too small\n");
    if((Sb = atoi(argv[3])) <= 0) error("Sb too small\n");
    filename = argv[1];
    if ((fd = TEMP_FAILURE_RETRY(open(filename, O_RDWR))) == -1)
		error("Cannot open file");
    initializeBuffers(buffer, Sb);
    fillaiostructs(aiocbs, buffer, fd, Sb);
    //printf("%s %d %d\n", filename, Nb, Sb);
    processblock(aiocbs, buffer, Nb, Sb);
    cleanUp(buffer);
    if (TEMP_FAILURE_RETRY(close(fd)) == -1)
		error("Cannot close file");
}