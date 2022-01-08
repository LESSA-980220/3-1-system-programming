#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 128 // the maximum length command
#define READ_END 0
#define WRITE_END 1

// to delete element in string
void delete_element(char *string, char element);

int main(void)
{
	char *args[MAX_LINE/2 + 1]; 		// command line arguments.
	char *args_pipe[MAX_LINE/2 + 1]; 	// arguments for pipe.
	int alive = 1; 						// flag to determine when to exit program.
	char buffer[MAX_LINE]; 				// sub string to make args.
	char cwd[MAX_LINE]; 					// to save current working directory.
	int background = 0, status; 		// background : to check background. status : to check error when do execvp.
	int fd;								// to save command to history file.
	pid_t c_pid, pid; 					// to fork.
	char *semi[2]; 						// to separate buffer by semi_colon (;) and save.

	// open history_file to save history.
	if ((fd = open("history_file.txt", O_CREAT | O_APPEND | O_RDWR, 0666)) == -1){
		perror("OPEN ERROR");
		exit(1);
	}

	while (alive) {
		int num = 0, pipe_index = 0;
		int num_semi=0;

		getcwd(cwd, MAX_LINE-1);
		printf("\n%s $", cwd);

		// clear all char variable and STDIN, STDOUT.
		memset(buffer, '\0', MAX_LINE);
		memset(args, '\0', MAX_LINE);
		memset(semi, '\0', MAX_LINE);
		fflush(stdin);
		fflush(stdout);

		// get command.
		fgets(buffer, sizeof(buffer), stdin);
		printf("\n");

		// save command to history_file.
		if (write(fd, buffer, strlen(buffer)) != strlen(buffer)){
			perror("WRITE HISTORY_FILE ERROR");
			exit(1);
		}

		// check how many semi_colon (;) is used.
		for (int i=0; i<strlen(buffer); i++){
			if (buffer[i] == ';')
				num_semi++;
		}

		// separate buffer by semi_colon (;).
		// to execute each buffer (separated by semi_colon) independently, use fork.
		for (int i=0; i<num_semi; i++){
			switch(c_pid = fork()){
				case -1:
					perror("SEMI_COLON FORK ERROR");
					break;
				case 0:
					semi[0] = strtok(buffer, ";");
					strcpy(buffer, semi[0]);
					break;
				default:
					waitpid(c_pid, NULL, 0);
					strtok(buffer, ";");
					semi[0] = strtok(NULL, "\n");
					strcpy(buffer, semi[0]);
					break;
			}
		}

		// check background
		if (buffer[strlen(buffer)-2] == '&'){ // If there are background sign &,
			background = 1;					   // Delete & sign and background = 1
			buffer[strlen(buffer)-2] = '\n';
			buffer[strlen(buffer)-1] = '\0';
		}

		// if command is non_semi_colon, add '\n' for executing well at below code.
		if (num_semi != 0)
			strcat(buffer, "\n");

		// if command is nothing, continue.
		if (strcmp(buffer, "cd \n") == 0 || strcmp(buffer, "cd\n") == 0 || buffer[0] == '\n'){
			continue;
		}
		else
			buffer[strlen(buffer) - 1] = '\0';

		// parse buffer to args.
		args[0] = strtok(buffer, " ");
		if (strcmp(args[0], "quit") == 0)
			break;

		while (args[num] != NULL){
			args[++num] = strtok(NULL, " ");
		}

		// cd command (internal)
		if (strcmp(args[0], "cd") == 0){
			if (strncmp(args[1], "/", 1) == 0){
				strcpy(cwd, args[1]);
				if (chdir(cwd) == -1){
					perror("CHDIR ERROR 1");
					exit(1);
				}
			} else{
				strcat(cwd, "/");
				strcat(cwd, args[1]);
				if (chdir(cwd) == -1){
					perror("CHDIR ERROR 2");
					exit(1);
				}
			}

			continue;
		}

		// history command (internal)
		if (strcmp(args[0], "history") == 0){
			lseek(fd, 0, SEEK_SET);
			while(read(fd, buffer, MAX_LINE-2) != 0){
				printf("%s", buffer);
				memset(buffer, 0, MAX_LINE);
			}

			continue;
		}

		// for other external command
		for (int i=0; i < num; i++){

			// redirection command
			if (strcmp(args[i], ">") == 0 | strcmp(args[i], "<") == 0 | strcmp(args[i], ">>") == 0){
				int fd;
				mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH;
				if (strcmp(args[i], ">") == 0){													// Case 1 : >
					if ((fd = open(args[i+1], O_CREAT | O_WRONLY | O_TRUNC, mode)) == -1){
						perror("OPEN ERROR AT >");
						break;
					}
					dup2(fd, STDOUT_FILENO);
					close(fd);
				}
				else if (strcmp(args[i], "<") == 0){											// Case 2 : <
					if ((fd = open(args[i+1], O_RDONLY, mode)) == -1){
						perror("OPEN ERROR AT <");
						break;
					}
					dup2(fd, STDIN_FILENO);
					close(fd);
				}
				else{																				// Case 3 : >>
					if ((fd = open(args[i+1], O_CREAT | O_WRONLY | O_APPEND, mode)) == -1){
						perror("OPEN ERROR AT >>");
						break;
					}
					dup2(fd, STDOUT_FILENO);
					close(fd);
				}

				args[i] = NULL;
				num -= 2;
				alive = 0;					// To check the change, stop program.
			}

			// pipe command.
			else if (strcmp(args[i], "|") == 0){
				pipe_index = i;
				args[i] = NULL;
				int k=0;

				// If command is A | B,	Make A -> args / B -> args_pipe.
				while (k < num - i - 1){
					args_pipe[k] = args[i + k + 1];
					if (args_pipe[k][0] == '"')
						delete_element(args_pipe[k], '"');
					else if (args_pipe[k][0] == '\'')
						delete_element(args_pipe[k], '\'');
					args[i + k + 1] = NULL;
					k++;
				}

				if (pipe_index != i)
					num++;
				num = num - k - 1;
				args_pipe[k] = NULL;
			}
		}

		// fork to execute command.
		pid = fork();

		if (pid < 0){							// fork error
			perror("FORK FAILED");
			return -1;
		}
		else if (pid == 0){					/// child
			// if command is pipe.
			if (pipe_index != 0){
				int pipe_fd[2];
				pid_t pid_pipe;

				if (pipe(pipe_fd) == -1){
					perror("PIPE FAILED");
					exit(1);
				}
				pid_pipe = fork();

				if (pid_pipe < 0){
					perror("PIPE FORK FAILED");
					exit(1);
				}
				else if (pid_pipe == 0){		// pipe_child
					close(pipe_fd[WRITE_END]);
					dup2(pipe_fd[READ_END], STDIN_FILENO);
					close(pipe_fd[READ_END]);
					status = execvp(args_pipe[0], args_pipe);
					if (status == -1){
						perror("FAIL TO EXECUTE THE PIPE COMMAND");
						exit(1);
					}
				}
				else {							// pipe_parent
					close(pipe_fd[READ_END]);
					dup2(pipe_fd[WRITE_END], STDOUT_FILENO);
					close(pipe_fd[WRITE_END]);
				}
			}

			status = execvp(args[0], args);
			if (status == -1){
				perror("FAIL TO EXECUTE THE COMMAND");
				exit(1);
			}
		}

		else{									// parent
			if (background){					// background, no wait.
				printf("PID #%d IS WORKING IN BACKGROUND : %s\n", pid, buffer);
			} else{							// foreground, wait child.
				waitpid(pid, NULL, 0);
			}
		}

		// clear variable for next loop.
		num = 0;
		background = 0;

		// if command was semi_colon, finish process.
		if (num_semi > 0){
			if (c_pid == 0)
				exit(1);
			else
				kill(c_pid, SIGKILL);
		}
	}

	return 0;
}

void delete_element(char *string, char element){
	for (; *string != '\0'; string++){
		if (*string == element){
			strcpy(string, string+1);
			string--;
		}
	}
}
