#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#include "builtins.h"
#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "signalLibrary.h"
#include "execLibrary.h"

int countSizeCom(command *com){
	int count = 0;

	argseq * argseq = com->args;
	do{
		count++;
		argseq= argseq->next;
	}while(argseq!=com->args);

	return count;
}

int countPipelineCom(pipelineseq * ln){
	int c = 0;
	pipeline *p = ln->pipeline;
	commandseq * commands= p->commands;
	
	if (commands==NULL){
		perror("ERROR in countPipelineCom(). commands==NULL\n");
		return -1;
	}
	do{
		c++;
		commands= commands->next;
	}while (commands!=p->commands);

	return c;
}

pipelineseq *parselineSafe(pipelineseq *ln, char *buf){
	ln = parseline(buf);
	if (!ln){
		perror("Parseline() was failed: returns null");
		perror(SYNTAX_ERROR_STR);
		exit(1);
	}
	return ln;
}

void findArgs(command *com, int sizeArgs, char **arguments){
	for(int i = 0; i < sizeArgs; i++)
		arguments[i] = com->args[i].arg;
		
	arguments[sizeArgs] = NULL;
}

int findDescriptors(command *com){
	struct redirseq *check = com->redirs;
	int curFlag, input_fd = 0, output_fd = 0, remmFlag = -1;
	while (check != NULL && check->r != NULL){
		curFlag = check->r->flags;
		if(remmFlag == curFlag)
			break;
		if(!input_fd && IS_RIN(curFlag)){
			input_fd = open(check->r->filename, O_RDONLY);
			if(input_fd == -1){
				fprintf(stderr, "%s: no such file or directory\n", check->r->filename);
            	return -1;
			}
			if (dup2(input_fd, STDIN_FILENO) == -1) {
				close(input_fd);
				perror("Error redirecting input\n");
				return -1;
			}
			close(input_fd);
		}
		if(!output_fd){
			if(IS_ROUT(curFlag)){
				output_fd = open(check->r->filename, O_CREAT | O_WRONLY | O_TRUNC, 00644);
				if(output_fd == -1){
      					fprintf(stderr, "%s: permission denied\n", check->r->filename);
					return -1;
				}
				if (dup2(output_fd, STDOUT_FILENO) == -1) {
					close(output_fd);
					perror("Error redirecting output\n");
					return -1;
				}
				close(output_fd);
			}
			if(IS_RAPPEND(curFlag)){
				output_fd = open(check->r->filename, O_CREAT | O_WRONLY | O_APPEND, 00644);
				if(output_fd == -1){
					if (access(check->r->filename, W_OK) == 0)
      					fprintf(stderr, "%s: permission denied\n", check->r->filename);
					return -1;
				}
				if (dup2(output_fd, STDOUT_FILENO) == -1) {
					close(output_fd);
					perror("Error redirecting output\n");
					return -1;
				}
				close(output_fd);
			}
		}
		if(input_fd && output_fd)
			return 0;
		remmFlag = curFlag;
		check = check->next;
	}
	return 0;
}

int isBackgroundProc(int flag){
	if(flag & INBACKGROUND == INBACKGROUND)
		return 1;
	return 0;
}

void execCommandInChild(command *com, pipeline *p, int counterDone, int fd[2][2], commandseq *commands, char **arguments){
	if(isBackgroundProc(p->flags) == 1)
		setsid();

	if(counterDone != 0){
		dup2(fd[(counterDone + 1) % 2][0], STDIN_FILENO);
		close(fd[(counterDone + 1) % 2][0]);
		close(fd[(counterDone + 1) % 2][1]);
	}
	if(commands->next != p->commands){
		dup2(fd[counterDone % 2][1], STDOUT_FILENO);
		close(fd[counterDone % 2][0]);
		close(fd[counterDone % 2][1]);
	}
	if(findDescriptors(com) != -1){
		struct sigaction sigAct = {
			.sa_handler = SIG_DFL,
			.sa_flags = 0
		};
		sigemptyset(&sigAct.sa_mask);
		sigaction(SIGINT, &sigAct, NULL);
		doTaskInChild(arguments);
	}
	else
		exit(EXEC_FAILURE);
	exit(0);
}

void execPipeline (command *com, pipeline *p){
	int counterDone = 0, in = 0, fd[2][2], sizeArgs;
	commandseq *commands= p->commands;

	if (commands==NULL){
		perror("ERROR in countPipelineCom(). commands==NULL\n");
		exit(1);
	}
	if(commands->next == p->commands){
		com = commands->com;
		int sizeArgs = countSizeCom(com);
		char* arguments[sizeArgs + 1];
		findArgs(com, sizeArgs, arguments);

		char *commandToDO = arguments[0];
		int indexOfCommand = isShellTask(commandToDO);
		if(indexOfCommand != -1 && counterDone == 0){
			doTaskInShell(indexOfCommand, arguments, sizeArgs);
			return;
		}
	}

	sigset_t setting1, setting2;
	sigemptyset(&setting1);
	sigfillset(&setting2);
	sigdelset(&setting2, SIGCHLD);
	sigaddset(&setting1, SIGCHLD);

	sigprocmask(SIG_BLOCK, &setting1, NULL);
	do{
		com = commands->com;
		int sizeArgs = countSizeCom(com);
		char* arguments[sizeArgs + 1];
		findArgs(com, sizeArgs, arguments);

		if(commands->next != p->commands)
			pipe(fd[counterDone % 2]);
		
		pid_t pid = fork();
		if (!pid){
			sigprocmask(SIG_UNBLOCK, &setting1, NULL);
			execCommandInChild(com, p, counterDone, fd, commands, arguments);
		}
		else if (pid < 0){
			perror("fork() was failed");
			exit(1);
		}
		else{
			if(!isBackgroundProc(p->flags)){
				addPid(pid ,1);
			}
			if(counterDone != 0){
				close(fd[(counterDone + 1) % 2][0]);
				close(fd[(counterDone + 1) % 2][1]);
			}
		}

		counterDone++;
		commands= commands->next;
	}while (commands != p->commands);

	if(!isBackgroundProc(p->flags)){
		while (foregroundProc.index > 0)
			sigsuspend(&setting2);
	}
	sigprocmask(SIG_UNBLOCK, &setting1, NULL);
}

void execCommand(char *buf, pipelineseq *ln){
	command *com;
	pipelineseq * ps = ln;

	if (!ln){
		fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
		return;
	}
	do{
		execPipeline(com, ps->pipeline);
		ps= ps->next;
	} while(ps!=ln);
}