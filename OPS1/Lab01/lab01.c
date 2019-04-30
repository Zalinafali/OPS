#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>

#define MAXFD 20
#define MAX_SIZE 99999999
#define MAX_PATH 30

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

int SMALLEST = MAX_SIZE; 
long int DEPTH = 0;
int save_flag = 1;
FILE* f = NULL;

int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
    if(f->level > DEPTH)
       return 0;
	if(FTW_F == type){
        if(save_flag){
            if(s->st_size == SMALLEST)
                printf("%s\tsize: %d\t - SMALLEST\n", name, s->st_size);
            else
                printf("%s\tsize: %d\n", name, s->st_size);
        }
        else{
            if(s->st_size == SMALLEST)
                fprintf(f,"%s\tsize: %d\t - SMALLEST\n", name, s->st_size);
            else
                fprintf(f,"%s\tsize: %d\n", name, s->st_size);
        }
    }
    return 0;
}

int walkSmallest(const char *name, const struct stat *s, int type, struct FTW *f)
{
	if(FTW_F == type){
        if(s->st_size < SMALLEST)
            SMALLEST = s->st_size;
    }
    return 0;
}

int main(int argc, char** argv) {
    char* path = getenv("PATH");
    char* lim = getenv("LIMIT");
    DEPTH = strtol(lim, (char **)NULL, 0);
    
    char* save = getenv("FILENAME");
    if((strcmp(save,"yes")) == 0){
        save_flag = 0;
        f = fopen(".sizes", "w+");
    }

    if(nftw(path,walkSmallest,MAXFD,FTW_DEPTH)!=0)
        printf("%s: brak dostepu\n",path);
	if(nftw(path,walk,MAXFD,FTW_PHYS)!=0)
        printf("%s: brak dostepu\n",path);

	return EXIT_SUCCESS;
}
