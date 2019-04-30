#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include<ctype.h>

#define MSG_LEN 100

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

void wrong_PID(int* peer_PID){
    perror("mq");
    printf("peer_PID set to 0\n");
    *peer_PID = 0;
}

void writing_ERR(int* peer_PID, mqd_t* mq_peer){
    mq_close(*mq_peer);
    printf("Close %d", *peer_PID);
    wrong_PID(peer_PID);
}

void sethandler( void (*f)(int, siginfo_t*, void*), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction = f;
	act.sa_flags=SA_SIGINFO;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void usage(void){
	fprintf(stderr,"USAGE: signals n k p l\n");
	fprintf(stderr,"100 >n > 0 - number of children\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if(argc!=1) usage();
    char name[20];
    if(snprintf(name,20,"/mq_%d",(int)getpid()) < 0) ERR("snprintf");
    mqd_t mq_PID;
    struct mq_attr attr;
	attr.mq_maxmsg= 4;
	attr.mq_msgsize= MSG_LEN;
    if((mq_PID=TEMP_FAILURE_RETRY(mq_open(name, O_RDWR | O_CREAT, 0606, &attr)))==(mqd_t)-1)
        ERR("mq open in");

    printf("PID: %d\n", (int)getpid());

    int peer_PID = 0;
    char peer_name[20];
    mqd_t mq_peer;
    char msg[MSG_LEN];
    for(;;){
        printf("New message\n");
        fgets(msg,MSG_LEN,stdin);
        if(isdigit(msg[0])){
            if(peer_PID) mq_close(mq_PID);
            peer_PID = atoi(msg);
            printf("peer_PID: %d\n", peer_PID);
            if(snprintf(peer_name,20,"/mq_%d",peer_PID) < 0) ERR("snprintf");
            if((mq_peer=TEMP_FAILURE_RETRY(mq_open(peer_name, O_WRONLY)))==(mqd_t)-1) wrong_PID(&peer_PID);
        }
        else{
            if(!peer_PID) printf("%s\n", msg);
            else{
                if(mq_send(mq_peer,msg,MSG_LEN,1)){
                    if(errno==EAGAIN) printf("Queue full");
                    writing_ERR(&peer_PID,&mq_peer);
                }
                printf("%d write to %d\n", getpid(), peer_PID);
            }
        }
    }
    mq_close(mq_peer);
    mq_close(mq_PID);
    if(mq_unlink(name))ERR("mq unlink");
}