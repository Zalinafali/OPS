#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>

#define BACKLOG 5
#define S_PORT 2000

#define ERR(source) (perror(source),\
                fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                exit(EXIT_FAILURE))

void usage(char * name){
        fprintf(stderr,"USAGE: %s socket port\n",name);
}

void Draw(int n, int x, int y){
    int i, j;
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(x == i && y == j)
                printf("[0]");
            else
                printf("[ ]");
        }
        printf("\n");
    }
    printf("\n");
}

int make_socket(){
	int sock;
	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock < 0) ERR("socket");
	return sock;
}

int bind_tcp_socket(){
        struct sockaddr_in addr;
        int socketfd,t=1;
        socketfd = make_socket();
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(S_PORT);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,&t, sizeof(t))) ERR("setsockopt");
        if(bind(socketfd,(struct sockaddr*) &addr,sizeof(addr)) < 0)  ERR("bind");
        if(listen(socketfd, BACKLOG) < 0) ERR("listen");
        return socketfd;
}

int add_new_client(int sfd){
        int nfd;
        if((nfd=TEMP_FAILURE_RETRY(accept(sfd,NULL,NULL)))<0) {
                if(EAGAIN==errno||EWOULDBLOCK==errno) return -1;
                ERR("accept");
        }
        return nfd;
}

void doServer(int fdT){
        int cfd,fdmax;
        fd_set base_rfds, rfds ;
        sigset_t mask, oldmask;
        FD_ZERO(&base_rfds);
        FD_SET(fdT, &base_rfds);
        fdmax=fdT;
        sigemptyset (&mask);
        sigaddset (&mask, SIGQUIT);
        sigprocmask (SIG_BLOCK, &mask, &oldmask);
        while(1){
                rfds=base_rfds;
                if(pselect(fdmax+1,&rfds,NULL,NULL,NULL,&oldmask)>0){
                        cfd=add_new_client(fdT);
                        if(cfd>=0)
                        printf("New client connected\n");
                }else{
                        if(EINTR==errno) continue;
                        ERR("pselect");
                }
        }
        sigprocmask (SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char **argv) {
    if(argc!=3) {
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    int n = atoi(argv[1]);
    if(n < 5 || n > 10){
        printf("n out of range\n");
        return EXIT_FAILURE;
    }
    int k = atoi(argv[2]);
    if(k < 3 || k > 10){
        printf("k out of range\n");
        return EXIT_FAILURE;
    }
    int x = n/2;
    int y = n/2;
    printf("n = %d\tk = %d\tx = %d\ty = %d\n", n, k, x, y);
    Draw(n,x,y);
    
    int fd, new_flags;
    fd = bind_tcp_socket();
    new_flags = fcntl(fd, F_GETFL) | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flags);
    doServer(fd);


    if(TEMP_FAILURE_RETRY(close(fd))<0)ERR("close");
    fprintf(stderr,"Server has terminated.\n");
    return EXIT_SUCCESS;
}