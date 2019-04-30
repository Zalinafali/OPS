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
        fprintf(stderr,"USAGE: %s domain port\n",name);
}

int make_socket(){
        int sock;
        sock = socket(PF_INET,SOCK_STREAM,0);
        if(sock < 0) ERR("socket");
        return sock;
}

struct sockaddr_in make_address(char *address, char *port){
        int ret;
        struct sockaddr_in addr;
        struct addrinfo *result;
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        if((ret=getaddrinfo(address,port, &hints, &result))){
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
                exit(EXIT_FAILURE);
        }
        addr = *(struct sockaddr_in *)(result->ai_addr);
        freeaddrinfo(result);
        return addr;
}

int connect_socket(char *name, char *port){
        struct sockaddr_in addr;
        int socketfd;
        socketfd = make_socket();
        addr=make_address(name,port);
        if(connect(socketfd,(struct sockaddr*) &addr,sizeof(struct sockaddr_in)) < 0){
                if(errno!=EINTR) ERR("connect");
                else {
                        fd_set wfds ;
                        int status;
                        socklen_t size = sizeof(int);
                        FD_ZERO(&wfds);
                        FD_SET(socketfd, &wfds);
                        if(TEMP_FAILURE_RETRY(select(socketfd+1,NULL,&wfds,NULL,NULL))<0) ERR("select");
                        if(getsockopt(socketfd,SOL_SOCKET,SO_ERROR,&status,&size)<0) ERR("getsockopt");
                        if(0!=status) ERR("connect");
                }
        }
        return socketfd;
}

int main(int argc, char** argv) {
        if(argc!=3) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        int fd;
        fd=connect_socket(argv[1], argv[2]);
        if(TEMP_FAILURE_RETRY(close(fd))<0)ERR("close");
        return EXIT_SUCCESS;
}