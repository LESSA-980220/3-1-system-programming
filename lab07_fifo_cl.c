#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLINE 1024
#define READ_END 0
#define WRITE_END 1

void fatal(const char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int fd[2];
    char buf[MAXLINE];

    if ((fd[READ_END] = open("./my_fifo_write", O_RDONLY)) == -1)
    	fatal("read open error");
    if ((fd[WRITE_END] = open("./my_fifo_read", O_WRONLY)) == -1)
    	fatal("write open error");

    for(;;){
    	fflush(stdout);
    	memset(buf, 0, MAXLINE);

    	if(read(fd[READ_END], buf, MAXLINE) < 0)
    		fatal("read error");
    	printf("[SERVER] > %s", buf);

    	printf("[CLIENT] > ");
    	fgets(buf, MAXLINE, stdin);
    	write(fd[WRITE_END], buf, MAXLINE);

    	if (strcmp(buf, "quit\n") == 0){
    		printf("\n\nClient quit, goodbye!\n");
    		break;
    	}

    	printf("\n");
    }

    close(fd[READ_END]);
    close(fd[WRITE_END]);

    return 0;
}
