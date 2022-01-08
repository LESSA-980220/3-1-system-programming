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
    int sfd, fd, n;
    ssize_t numRead;
    char buf[BUF_SIZE];
    char command[BUF_SIZE], filename[BUF_SIZE], file_contents[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        fatal("socket");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        fatal("connect");

    for(;;){

    	// Char 배열 초기화
		memset(buf, '\0', sizeof(char) * BUF_SIZE);
		memset(command, '\0', sizeof(char) * BUF_SIZE);
		memset(filename, '\0', sizeof(char) * BUF_SIZE);
		memset(file_contents, '\0', sizeof(char) * BUF_SIZE);

		printf("'start' : show filename\n");
		printf("else : enter file name you want\n\n");
		printf("\nEnter your command below\n");

		// Command 입력
		if ((numRead = read(STDIN_FILENO, command, BUF_SIZE)) <= 0)
			fatal("READ COMMAND ERROR");

        // command가 start (file name list를 요구)
		if (strcmp(command, "start\n") == 0){
			strcpy(buf, "start");
			printf("\n-------------------------------------------------\n\n");

			// start command를 Server에게 전송
			if (write(sfd, buf, numRead) != numRead)
			   fatal("WRITE ERROR 1");

			// Server로부터 받아온 file name list buf를 출력
			numRead = read(sfd, buf, BUF_SIZE);
			if (write(STDOUT_FILENO, buf, numRead) != numRead)
				fatal("WRITE ERROR 2");

			printf("\nSuccess to load file name list\n");
			printf("\n-------------------------------------------------\n\n");

		} else{	// 원하는 filename 혹은 그 외의 값을 Server에게 전송
			strcpy(buf, command);

			// command를 Server에게 전송
			if (write(sfd, buf, numRead) != numRead)
			   fatal("WRITE ERROR 3");

			// 같은 기기, 같은 Directory 내에서 Server와 Client를 모두 실행함으로 인해
			// Client가 test.txt를 Server로부터 받아와서 test.txt를 만든다면
			// 기존의 test.txt와 파일명이 겹치므로 Client가 생성하는 file name을 test.txt_2로 설정 (copy본 처리)
			buf[strlen(buf)-1] = '\0';
			strcat(buf, "_2");
			strcpy(filename, buf);

			// Server로부터 file 내용을 받아옴
			n = read(sfd, file_contents, BUF_SIZE);

			// 만약 존재하지 않는 filename을 전송했을 경우 Server로부터 quit 메세지를 받아 NO FILE ERROR 오류 처리
			if (strcmp(file_contents, "quit") == 0){
				printf("\n");
				fatal("NO FILE ERROR");
			}

			// file 생성
			fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);

			file_contents[n] = '\0';

			// 생성한 file에 Server로부터 받아온 file 내용을 옮김
			if(write(fd, file_contents, strlen(file_contents)) == -1)
				fatal("WRITE ERROR 4");

			printf("\nSuccess to make %s file\n", filename);
			printf("\n-------------------------------------------------\n\n");

		}

	}
    exit(EXIT_SUCCESS);
}

void fatal(const char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}
