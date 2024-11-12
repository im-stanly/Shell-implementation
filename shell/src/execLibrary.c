#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "builtins.h"

int isShellTask(char *task){
	char *operationName = builtins_table[0].name;
	for(int i = 0; operationName != NULL; i++){
		if(strcmp(operationName, task) == 0)
			return i;
		operationName = builtins_table[i + 1].name;
	}
	return -1;
}

void doTaskInShell(int indexOfCommand, char **arguments, int argsNumber){
	errno = 0;
	builtins_table[indexOfCommand].fun(arguments);

	switch (errno){
	case 0:
		break;
	case 1:
		fprintf(stderr, "%s: permission denied\n", arguments[0]);
		break;
	case 2:
		fprintf(stderr, "%s: no such file or directory\n", arguments[0]);
		break;
	default:
		fprintf(stderr, "Builtin %s error.\n", arguments[0]);
		break;
	}
}

void doTaskInChild(char **arguments){
	execvp(arguments[0], arguments);

	switch (errno){
	case 1:
		fprintf(stderr, "%s: permission denied\n", arguments[0]);
		exit(EXEC_FAILURE);
	case 2:
		fprintf(stderr, "%s: no such file or directory\n", arguments[0]);
		exit(EXEC_FAILURE);
	case 13:
		fprintf(stderr, "%s: permission denied\n", arguments[0]);
		exit(EXEC_FAILURE);
	default:
		if(errno != 0){
			fprintf(stderr, "Builtin %s error\n", arguments[0]);
			exit(EXEC_FAILURE);
		}
	}
}