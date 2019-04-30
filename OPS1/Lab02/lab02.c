#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ftw.h>
#include <time.h>
#include <string.h>

#define MAX 256

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))


volatile sig_atomic_t last_signal=0;

void sethandler(void (*f)(int), int sigNo)
{	
	struct sigaction a;
	memset(&a, 0, sizeof(struct sigaction));
	a.sa_handler =f;
	sigaction(sigNo, &a, NULL);
}

void sig_handler(int sig)
{
   // if(SIGUSR1 == sig) printf("-");
  //  if(SIGUSR2 == sig) printf(".");
	last_signal = sig;
}

void sigchld_handler(int sig)
{
	pid_t p;
	while(1)
	{
		p=waitpid(0,NULL,WNOHANG);
		if(p<0) return;
	}
}

void child_work(char code, char* t_str)
{
    int t = atoi(t_str);
	struct timespec tt = {0,t*1000000};
	
	printf("[%d] code: %c; time: %d\n", getpid(),code,t);
	//printf("sth\n");
    while(1){
	    nanosleep(&tt,NULL);
	    if(code == '-') kill(getppid(), SIGUSR1);
	    if(code == '.') kill(getppid(), SIGUSR2);
    }
}

void parent_work(sigset_t oldmask)
{
	char buf[5]={"     "};
    char current;
    int ch;
	while(1)
	{

		last_signal=0;
		while(last_signal != SIGUSR2 && last_signal != SIGUSR1) sigsuspend(&oldmask);

		if(last_signal==SIGUSR1) current='-';
		if(last_signal==SIGUSR2) current='.';
		
		buf[4]=buf[3];
		buf[3]=buf[2];
		buf[2]=buf[1];
		buf[1]=buf[0];
		buf[0]=current;

        for(j=0;j<len-5;j++)
		{
			ch=0;
			if(buf[0]==tmp[j]-48)ch++;
			if(buf[1]==tmp[j+1]-48) ch++;
			else ch=0;
				
			if(buf[2]==tmp[j+2]-48)ch++;
			else ch=0;
					
			if(buf[3]==tmp[j+3]-48) ch++;
			else ch=0;
						
			if(buf[4]==tmp[j+4]-48) ch++;
			else ch=0;
							
			if(lch<ch)
			{
				printf("longer substring: %d\n",ch);
				lch=ch;
			}
			
			
		}
		
		if (ch==5) 
		{
			break;
			exit(EXIT_SUCCESS);
		}
		
		printf("[%c %c %c %c %c]\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
	}
}

int main (int agrc, char** argv)
{
    if(agrc != 3) ERR("agrc");
	int i, Fd, Fm;
	ssize_t len_Fm, len_Fd;
	char *tmp_Fm=malloc(MAX);
    char *tmp_Fd=malloc(MAX);
	pid_t pid;
	
	sethandler(sigchld_handler,SIGCHLD);
	sethandler(sig_handler,SIGUSR1);
	sethandler(sig_handler,SIGUSR2);

    sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	Fm=open(argv[1], O_RDONLY);
	len_Fm=read(Fm,tmp_Fm,MAX);

    Fd=open(argv[2], O_RDONLY);
	len_Fd=read(Fd,tmp_Fd,MAX);
	
	
	tmp_Fd = strtok(tmp_Fd, " \n");

	for(i=0; tmp_Fd != NULL; i++)
	{
        //if(NULL == tmp_Fd) ERR("fork");
		pid=fork();
		
		if(pid==0)
		{
            //printf("char:%c \ntime:%s\n", tmp_Fm[i], tmp_Fd);
			child_work(tmp_Fm[i], tmp_Fd);
            free(tmp_Fm);
            free(tmp_Fd);
            close(Fm);
            close(Fd);
			exit(EXIT_SUCCESS);
		}
        else{
		    tmp_Fd = strtok(NULL, " \n");
        }
    }
    parent_work(oldmask);
	
    free(tmp_Fm);
    free(tmp_Fd);
    close(Fm);
    close(Fd);
	
	exit(EXIT_SUCCESS);
}
