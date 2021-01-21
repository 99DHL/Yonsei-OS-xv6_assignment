#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define buf_size 128    // Max length of user input
#define max_args 8      // Max number of arguments per process

// Read a shell input.
char* readcmd(char *buf) {
    // Read input from stdin.
    fprintf(stdout, "$ ");
    memset(buf, 0, buf_size);
    char *cmd = fgets(buf, buf_size, stdin);

    // Chop the tailing \n.
    cmd[strlen(cmd)-1] = 0;
    return cmd;
}

// handle pipe commands. (implemented in a recursive way)
void handle_pipe(int i, int j, char **argv)
{
	while(j <= (i - 1))
	{	
		char *target = argv[j];
		while(*target != '\0')
		{
			if(*target == '|')
				break;
			target++;
		}

		if(*target == '|')
		{
			int filedes[2];
			pipe(filedes);
	
			int rc = fork();
			if(rc < 0)
			{
				fprintf(stderr, "fork() in pipe failed\n"); exit(1);
			}
			else if(rc > 0)			// parent process --> execute the second part of divided command.
			{
				int k = 0;
				dup2(filedes[0], 0);
				close(filedes[1]);
			
				target++;
				if(*(target) != ' ' && *(target) != '\0')
				{
					argv[k] = target;
					k++;
				}

				j++;
				while(j <= (i-1))
				{
					argv[k] = argv[j];
					k++; j++;
				}
				argv[k] = NULL;
				handle_pipe(k, 0, argv);
			}
			else					// child process --> execute the first part of divided command.
			{
				dup2(filedes[1], 1);
				close(filedes[0]);

				target--;
				if(*(target) != ' ' && *(target) != '\0')
				{
					*(++target) = '\0';
					argv[++j] = NULL;
				}
				else
				{
					argv[j] = NULL;
				}

				if(execvp(argv[0], argv) == -1)
				{
					fprintf(stderr, "Command not found: %s\n", argv[0]);
					exit(1);
				}
			}
			break;
		}
		j++;
	}
}

// Run a command.
int runcmd(char *cmd) {
	char *argv[max_args];
 	int single_flag = 0;
   
	// Remove leading white space(s).
    while(*cmd == ' ') { cmd++; }

    // cd command
    if(!strncmp(cmd, "cd ", 3)) {
		// handle pipe and serial commands
		while(*(cmd+3) == ' '){
			cmd++;
		}
		char *i = cmd + 3;
		int flag = 0;

		while(*i != '\0') {
			if(*i == ';') {
				*i = '\0';
				flag = 1;
				break;
			}
			i++;
		}

		char *j = i;
		while(*(--j) == ' '){
			*j = '\0';
		}

        if(chdir(cmd+3) < 0) { fprintf(stderr, "Cannot cd %s\n", cmd+3); }
		if(flag == 1)
			runcmd(++i);	// execute commands coming after "cd" command.
    }
    // exit command: return -1 to terminate the while loop in main.
    else if(!strncmp(cmd, "exit", 4)) { 
		// when input is like "exitsdffd" --> Command not found: exitsdfd
		if(*(cmd+4) != '\0' && *(cmd+4) != ';' && *(cmd+4) != ' '){
			fprintf(stderr, "Command not found: %s\n", cmd);
			return 0;
		}

		while(*(cmd+4) == ' '){
			cmd++;
		}
		// when input is like "exit sdfssfk" --> Invalid Syntax
		if(*(cmd+4) != '\0' && *(cmd+4) != ';'){
			fprintf(stderr, "Invalid Synatx\n");
			return 0;
		}
		return -1; 
	}
    else {
		int k = 0;
		int semicolon = 0;
		char temp[buf_size];
		strcpy(temp, cmd);
		char *cmd_temp = temp;

		// handle serialized commands
		for(; *cmd_temp != '\0'; cmd_temp++)
		{
			if(*cmd_temp == ';')
			{
				single_flag = 1;
				*cmd_temp = '\0';
				*(cmd + semicolon) = '\0';
				k = 1;
				while(*(cmd+semicolon+k) != '\0')
				{
					*(cmd+semicolon+k) = '\0';
					k++;
				}
				runcmd(cmd);
				runcmd(++cmd_temp);
			}	
			semicolon++;
		}

		if(single_flag == 0)	// if no serialized commands left
		{
			int rc = fork();
			if(rc < 0)
			{
				fprintf(stderr, "fork() failed\n"); exit(1);	// if the command is not valid, the process must be killed.
			}
			else if(rc > 0)
			{
				wait(NULL);
			}
			else
			{
				int i = 0; int j = 0;
				argv[0] = strtok(cmd, " ");
				while(argv[i] != NULL)
				{
					i++;
					argv[i] = strtok(NULL, " ");
				}
	
				handle_pipe(i, j, argv);			// handle pipe commands

				if(execvp(argv[0], argv) == -1)
				{
					fprintf(stderr, "Command not found: %s\n", argv[0]);
					exit(1);	// if the command is not valid, the process must be killed.
				}
			}
		}
	}

	return 0;
}

int main(void) {
    int fd = 0;
    char *cmd = 0;
    char buf[buf_size];

    // Ensure three file descriptors are open. Failure will hang the shell.
    while((fd = open("console", O_RDWR)) >= 0) {
        if(fd >= 3) { close(fd); break; }
    }

    fprintf(stdout, "EEE3535 Operating Systems: starting ysh\n");

    // Read and run input commands.
    while((cmd = readcmd(buf))) {
        // Run the input commands.
        if(*cmd && runcmd(cmd)) { break; }
    }

    return 0;
}

