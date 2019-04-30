#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define DEFAULT_VECTOR 20
#define DEFAULT_TCOUNT 10
#define randint(start,end) (start + ((int)rand()%end))
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

typedef struct workers {
	pthread_t tid;
    int* GUESSED_VALUE;
    int* pVector;
    int vectorSize;
   //int seed;
    //sigset_t *pMask;
    pthread_mutex_t *pmxVector;
    pthread_mutex_t *pmxGUESSED_VALUE;
} workers_t;

void ReadArguments(int argc, char** argv, int *vectorSize, int* threadCount);
void* Work(void *voidPtr);

int main(int argc, char** argv) {
    int GUESSED_VALUE = 0;
    int vectorSize, threadCount;
    ReadArguments(argc,argv,&vectorSize,&threadCount);
    int* vector = (int*)malloc(sizeof(int)*vectorSize);
    if(NULL == vector) ERR("Malloc error for vector!");
    for(int i = 0; i < vectorSize; i++) vector[i] = 0;
    workers_t* workers = (workers_t*) malloc(sizeof(workers_t) * threadCount);
    if(NULL == workers) ERR("Malloc error for vector!");

    sigset_t mask, oldmask;
    sigemptyset(&mask);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    srand(time(NULL));
    pthread_mutex_t mxVector = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxGUESSED_VALUE = PTHREAD_MUTEX_INITIALIZER;
    for (int i = 0; i < threadCount; i++) {
        workers[i].GUESSED_VALUE = &GUESSED_VALUE;
        //workers[i].seed = rand();
        workers[i].pVector = vector;
        workers[i].vectorSize = vectorSize;
        workers[i].pmxVector = &mxVector;
        workers[i].pmxGUESSED_VALUE = &mxGUESSED_VALUE;
		int err = pthread_create(&(workers[i].tid), NULL, Work, &workers[i]);
		if (err != 0) ERR("Couldn't create thread");
    }
    int signo;
    for (;;) {
		if(sigwait(&mask, &signo)) ERR("sigwait failed.");
		switch (signo) {
			case SIGINT:
				pthread_mutex_lock(&mxVector);
				pthread_mutex_unlock(&mxVector);
                break;
            case SIGALRM:
                pthread_mutex_lock(&mxVector);
                for(int i = 0; i < vectorSize; i++) printf("%d ", vector[i]);
                printf("\n");
                setTimer(0.5);
                pthread_mutex_unlock(&mxVector);
                break;
			default:
				printf("unexpected signal %d\n", signo);
				exit(1);
		}
	}
    for (int i = 0; i < threadCount; i++) 
		if(pthread_join(workers[i].tid, NULL)) ERR("Failed to join with a worker thread!");
    free(vector);
    free(workers);
    exit(EXIT_SUCCESS);
}

void ReadArguments(int argc, char** argv, int *vectorSize, int* threadCount)
{
	*vectorSize = DEFAULT_VECTOR;
    *threadCount = DEFAULT_TCOUNT;

	if (argc >= 2) {
		*vectorSize = atoi(argv[1]);
		if (vectorSize <= 0) {
			printf("Invalid value for 'vectorSize size'");
			exit(EXIT_FAILURE);
		}
	}
    if (argc >= 3) {
		*threadCount = atoi(argv[2]);
		if (*threadCount <= 0) {
			printf("Invalid value for 'threadCount'");
			exit(EXIT_FAILURE);
		}
	}
}

void* Work(void *voidPtr) {
	workers_t *args = voidPtr;
	//printf("%ld\n", args->tid);
    int number;
    while(true){
        pthread_mutex_lock(args->pmxVector);
        printf("IN %ld\n", args->tid);
        //number = args->pVector[(rand_r(args->seed))%(args->vectorSize)];
        number = randint(0,args->vectorSize - 1);
        pthread_mutex_lock(args->pmxGUESSED_VALUE);
        if(*(args->GUESSED_VALUE) == args->pVector[number]) printf("BINGO!\n");
        if(args->pVector[number] > 0) *(args->GUESSED_VALUE) = args->pVector[number];
        pthread_mutex_unlock(args->pmxGUESSED_VALUE);
        printf("OUT %ld\n", args->tid);
        pthread_mutex_unlock(args->pmxVector);
        sleep(1);
    }
    return NULL;
}