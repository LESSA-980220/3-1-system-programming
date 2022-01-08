#include <fcntl.h>
#include <string.h> // 추가
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> // 추가
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

void creatEnv();

/**
 * 파일을 복사하여 사본을 생성한다.
 * @param buf   경로명을 저장할 버퍼 변수.
 * @param size  버퍼 변수의 크기
 * @return 버퍼 변수의 시작 주소, 즉 buf
 */

char *mygetcwd(char *buf, size_t size) {
	struct dirent *dp; // #include <dirent.h>를 6줄에 선언
	struct stat file_info;
	DIR* dirp;
	ino_t inode;
	char sub_buf1[255];
	char sub_buf2[255];

	for (;;){
		dirp = opendir("..");
		if (dirp == NULL)
			break;
		stat(".", &file_info);
		inode = file_info.st_ino;
		for (;;){
			if ((dp = readdir(dirp)) == NULL){
				break;
			}
			if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
				continue;
			if (inode == dp->d_ino){
				buf = strcat(strcat(dp->d_name, "/"), buf);
				chdir("..");
			}
		}
	}

	// 49 - 51줄은 "/" (슬래쉬) 기호를 정상적인 path 출력형태에 알맞게 조정하기위한 과정
	// ex) a/b/c/ -> /a/b/c
	strcpy(sub_buf2, buf);
	sprintf(sub_buf1, "/%s", sub_buf2);
	strncpy(buf, sub_buf1, strlen(sub_buf1)-1);
	// 이 과정을 위해 #include <string.h>를 2줄에 선언

	chdir(buf);
	return buf;
}

int main(void) {
  // pid_t pid; // 사용하지 않음
  // int status; // 사용하지 않음
  char buf[255];

  creatEnv();
  chdir("dir/sub");

  printf("original func: %s\n", getcwd(NULL, 0));
  printf("mygetcwd func: %s\n\n", mygetcwd(buf, 255));
  // 69줄은 mygetcwd 함수 작동이 끝난 뒤 작업 디렉토리가 원래의 경로로 정상 복구 되었나를 확인
  printf("after mygetcwd func working, getcwd\n : %s\n", getcwd(NULL, 0));

  return 0;
}

void creatEnv(){
  mkdir("dir", 0755);
  mkdir("dir/sub", 0755);
  mkdir("dir/sub2", 0);

  creat("dir/a", 0755);
  creat("dir/b", 0755);
  creat("dir/sub/x", 0755);
  symlink("dir/a", "dir/sl");
  symlink("dir/x", "dir/dsl");
}
