#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <fcntl.h>

#define SV_SOCK_PATH "./unix_xfr"
#define BUF_SIZE 1024

#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>

#define BACKLOG 5

void fatal(const char*);

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd, cfd, fd;
    ssize_t numRead; 
    char buf[BUF_SIZE];
    char command[BUF_SIZE], filename[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        fatal("socket");

    /* Construct server socket address, bind socket to it,
       and make this a listening socket */

    /* For an explanation of the following check, see the errata notes for
       pages 1168 and 1172 at http://www.man7.org/tlpi/errata/. */

    if (strlen(SV_SOCK_PATH) > sizeof(addr.sun_path) - 1){
			fprintf(stderr, "Server socket path too long: %s", SV_SOCK_PATH);
			exit(EXIT_FAILURE);
		}

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT){
			fprintf(stderr, "remove-%s", SV_SOCK_PATH);
			exit(EXIT_FAILURE);
		}

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        fatal("bind");

    if (listen(sfd, BACKLOG) == -1)
        fatal("listen");

    cfd = accept(sfd,  NULL, NULL);
    if (cfd == -1)
        fatal("accept");

    for (;;) {

    	 // Char 배열 초기화
		 memset(buf, '\0', sizeof(char) * BUF_SIZE);
		 memset(filename, '\0', sizeof(char) * BUF_SIZE);
		 memset(command, '\0', sizeof(char) * BUF_SIZE);

		 // Client로부터 command를 받음
        printf("Waiting for Client...\n\nCommand : ");
        recv(cfd, buf, BUF_SIZE, 0);
        strcpy(command, buf);

        printf("%s\n", command);

        // command가 start (file name list를 요구)
        if (strncmp(command, "start", 5) == 0){
        	DIR *dp;
        	struct dirent *dir;

        	printf("\nSend filename to Client\n\n\n\n");

        	// my_sv.c이 위치하는  Directory의 dp를 생성
			if((dp = opendir(".")) == NULL){
				fprintf(stderr,"directory open error\n");
				exit(-1);
			 }

			strcpy(buf, "");

			// my_sv.c이 위치하는 Directory 내에 존재하는 모든 file name을 불러옴
			while((dir = readdir(dp)) != NULL){
				if(dir->d_ino == 0) continue;
				if((strcmp(dir->d_name, ".") == 0) || (strcmp(dir->d_name, "..") == 0))
						continue;
				strcat(buf, dir->d_name);
				strcat(buf, "\n");
			}

			// file name buf를 Client로 전송
			if (write(cfd, buf, strlen(buf)) != strlen(buf))
				fatal("WRITE ERROR 1");

        } else {		// Client로부터 filename을 받거나 그 외의 값을 받음
        	printf("\nSend file to Client\n\n\n\n");

        	strncpy(filename, buf, strlen(buf)-1);

        	// filename에 해당하는 file을 열고, 만약 없다면 Client에게 종료를 위한 quit 메세지를 전송한 뒤 오류 처리
        	if((fd = open(filename, O_RDWR)) == -1){
    			if (write(cfd, "quit", 4) != 4)
    				fatal("WRITE ERROR 2");
        		fatal("NO FILE ERROR");
        	}

        	memset(buf, '\0', sizeof(char) * BUF_SIZE);

        	// file의 내용을 buf에 담음
        	if ((read(fd, buf, BUF_SIZE)) == -1)
        		fatal("READ ERROR");

        	// buf를 Client에게 전송
			if (write(cfd, buf, strlen(buf)) != strlen(buf))
				fatal("WRITE ERROR 3");
        }
    }
    return 0;
}

void fatal(const char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}
