#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv){
	int i;
	struct stat file_info;
	mode_t file_mode;

	for (i=1; i<argc; i++){

		if (stat(argv[i], &file_info) < 0){
			perror("stat : ");
			return -1;
		}
		file_mode = file_info.st_mode;
		if (S_ISDIR(file_mode)){
			if (chmod(argv[i], S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) < 0){
				perror("chmod error 1 : ");
			}
		} else{
			if (((file_mode & S_IXUSR) | (file_mode & S_IXGRP) | (file_mode & S_IXOTH)) != 0){
				if (chmod(argv[i], S_IRUSR | S_IRGRP | S_IROTH | S_IXUSR | S_IXGRP | S_IXOTH) < 0){

					perror("chmod error 2 : ");
				}
			} else{
				if (chmod(argv[i], S_IRUSR | S_IRGRP | S_IROTH) < 0){
					perror("chmod error 3 : ");
				}
			}
		}
	}

	return 0;
}
