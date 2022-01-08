#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#define MAXARGS 31
#define MAXFILENAME 1024
#define MAXPATHSTR 2048

extern char **environ;
/**
 * 첫 번째 인자인 file 프로그램을 뒤에 따르는 인자들을 이용해 실행한다.
 * @param file	실행할 프로그램의 이름
 * @param args	stdarg(3)을 통해 처리할 인자들
 * @return 실패했을 경우 -1, 성공 시 반환 값 없음
 */
int myexeclp(const char *file, const char *args, ...){
	va_list ap;
	char *array[MAXARGS];
	char *path = getenv("PATH");
	char *sub_path[MAXPATHSTR];
	char buf[MAXPATHSTR];

	int num = 0;

	va_start(ap, args);
	while(args != NULL){
		array[num++] = args;
		args = va_arg(ap, char *);
	}
	va_end(ap);

	num = 0;

	sub_path[0] = strtok(path, ":");
	while (sub_path[num] != NULL){
		sub_path[++num] = strtok(NULL, ":");
	}

	int err_code = 0;
	for (int k=num-1; k >= 0; k--){
		strcpy(buf, sub_path[k]);
		strcat(strcat(buf, "/"), file);

		if (execve(buf, array, environ) != -1){
			err_code = 1;
			break;
		}
	}

	if (err_code == 0){
		fprintf(stderr, "ERROR : No file or Can't execute\n");
		return -1;
	}
	return 0;
}

int main(void) {
  char path[MAXPATHSTR], filename[MAXFILENAME] = "hello";
  sprintf(path, "PATH=%s:%s", getcwd(NULL, 0), getenv("PATH"));
  putenv(path);

  // prepare the executable file named "hello"
  system("gcc -o hello hello.c");

  myexeclp(filename, "hello", "-a", "-b", "-c", (char *) 0);

  return 0;
}
