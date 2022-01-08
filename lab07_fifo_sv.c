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

    mkfifo("./my_fifo_read", S_IRUSR | S_IWUSR);
    mkfifo("./my_fifo_write", S_IRUSR | S_IWUSR);

    if ((fd[READ_END] = open("./my_fifo_read", O_RDWR)) == -1)
    	fatal("read open error");
    if ((fd[WRITE_END] = open("./my_fifo_write", O_RDWR)) == -1)
    	fatal("write open error");

    for(;;){
    	fflush(stdout);
    	memset(buf, 0, MAXLINE);

    	printf("[SERVER] > ");
    	fgets(buf, MAXLINE, stdin);

    	if (write(fd[WRITE_END], buf, MAXLINE) < 0)
    		fatal("write error");

    	if (read(fd[READ_END], buf, MAXLINE) < 0)
    		fatal("read error");

    	if (strcmp(buf, "quit\n") == 0){
    		printf("\n\nClient quit, goodbye!\n");
    		break;
    	}

    	printf("[CLIENT] > %s\n", buf);
	}
    return 0;
}
