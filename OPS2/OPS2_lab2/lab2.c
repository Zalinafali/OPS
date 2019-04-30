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
#include <fcntl.h>


#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                     exit(EXIT_FAILURE))
#define MSG_LEN 128

typedef struct bla
{
    mqd_t* other;
    int* otherLen;
    mqd_t q;
} bla;

void send_message(mqd_t* other, char* message, int otherLen)
{
    for(int i=0;i<otherLen;i++)
    {
        if(mq_send(other[i],message,MSG_LEN,0))ERR("mq_send");
    }
}


void notify_f(union sigval sv)
{
    bla b = *((bla*)sv.sival_ptr);
    char message[MSG_LEN];
    
    static struct sigevent not;
    not.sigev_notify=SIGEV_THREAD;
    not.sigev_notify_function = notify_f;
    not.sigev_notify_attributes = NULL;
    not.sigev_value.sival_ptr = sv.sival_ptr;

    if(mq_notify(b.q,&not)==-1) ERR("notify");

    unsigned msg_prio;
    for(;;)
    {
        
        if(mq_receive(b.q,message,MSG_LEN,&msg_prio)<1) 
        {
                if(errno==EAGAIN) break;
                else ERR("mq_receive");
        }
        
        printf("Read %s\n",message);
        

        send_message(b.other,message,*(b.otherLen));
    }
                
}

mqd_t create_queue(char* qname,bla* b)
{
    mqd_t queue;

    struct mq_attr attr;
    attr.mq_maxmsg=8;
    attr.mq_msgsize=MSG_LEN;
    char path[20]="";
    snprintf(path,20,"/%d_in",getpid());
    printf("Queue path: %s\n",path);
    queue=TEMP_FAILURE_RETRY(mq_open(path, O_RDWR | O_NONBLOCK | O_CREAT, 0606, &attr));
    if(queue==(mqd_t)-1) ERR("mq open");
    strcpy(qname,path);

    b->q=queue;
    struct sigevent not;
    not.sigev_notify=SIGEV_THREAD;
    not.sigev_notify_function = notify_f;
    not.sigev_notify_attributes = NULL;
    not.sigev_value.sival_ptr = b;  
    if(mq_notify(queue,&not)==-1) ERR("notify");

    return queue;
}




int main(int argc, char** argv) 
{
    char qname[20];
    char r;
    char message[128];
    mqd_t other[2];
    int otherLen = 0;
    struct mq_attr attr;
    char path[128];
    int cont = 1;

    bla* b = (bla*)malloc(sizeof(bla));
    if(!b) ERR("malloc");
    b->other = other;
    b->otherLen = &otherLen;
    mqd_t queue = create_queue(qname,b);

    while(cont)
    {
        scanf("%c",&r);
        for(int i=0;i<128;i++)
            message[i]=0;
        scanf("%c",&r);
        switch (r)
        {
            case 'd':
                scanf("%s",message);
                if(otherLen==2)
                {
                    mq_close(other[0]);
                    other[0]=other[1];
                    otherLen=1;
                }
                strcpy(path,"/");
                strcat(path,message);
                strcat(path,"_in");
                printf("Open %s\n",path);
                other[otherLen] = TEMP_FAILURE_RETRY(mq_open(path,O_RDWR|O_NONBLOCK,0600,&attr));
                if(other[otherLen]==(mqd_t)-1) ERR("mq open");
                otherLen++;
                break;

            case 'm':
                scanf("%s",message);
                printf("message: %s\n",message);
                send_message(other,message,otherLen);
                break;

            case 'c': //close without C-c
                cont=0;
        }

    }
    
    for(int i=0;i<otherLen;i++)
        mq_close(other[i]);
    mq_close(queue);
    mq_unlink(qname);
    free(b);

    return EXIT_SUCCESS;
}
