#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>

#include "builtins.h"

int echo(char*[]);

builtin_pair builtins_table[]={
	{"exit",	&exitShell},
	{"lecho",	&echo},
	{"lcd",		&cdShell},
	{"lkill",	&killShell},
	{"lls",		&lsShell},
	{NULL,NULL}
};

int stringToInt(char * argv){
	char * pEnd;
    long stringNumber = strtol(argv, &pEnd, 10);
    
	if((pEnd != NULL && pEnd[0] != '\0') || (stringNumber == 0 && argv[0] != '0'))
		return -1;
	if(errno == ERANGE || stringNumber > INT_MAX || stringNumber < INT_MIN)
        return -1;

	return (int) stringNumber;
}

int exitShell(char * argv[]){
    if(argv[1]){
        int exit_code = stringToInt(argv[1]);
        if(exit_code == -1){
            errno = EIO;
            exit(1);
        }
        exit(exit_code);
    } 
    else
        exit(0);
    return 0;
}

int cdShell(char *argv[]){
    if(argv[1] && argv[2]){
        errno = EIO;
        return 1;
    }
    else if (argv[1]){
        if (chdir(argv[1]) != 0){
            errno = EIO;
			return 1;
        }
    }
	else{
		const char *home_dir = getenv("HOME");
        if (home_dir == NULL) {
            errno = ENOENT;
            return 1;
        }

        if (chdir(home_dir) != 0) {
            errno = ENOENT;
            return 1;
        }
    }
    return 0;
}

int killShell(char *argv[]) {
    int signal_number = SIGTERM; 
    int pid;
    char *endptr;
    long result;
    argv++;

    if (argv[0] == NULL) {
        errno = EIO;
        return 1;
    }

    if (argv[0][0] == '-') {
        errno = 0;
        result = strtol(argv[0] + 1, &endptr, 10);
        if (*endptr != '\0' || errno != 0 || result <= 0 || result > INT_MAX) {
            errno = EIO;
            return 1;
        }
        signal_number = (int) result;
        argv++;
    }

    if (argv[0] == NULL) {
        errno = EIO;
        return 1;
    }

    errno = 0;
    result = strtol(argv[0], &endptr, 10);
    if (*endptr != '\0' || errno != 0 || result > INT_MAX || result < INT_MIN) {
        errno = EIO;
        return 1;
    }

    pid = (int) result;

    if (kill(pid, signal_number) < 0) {
        errno = EIO;
        return 1;
    }

    return 0;
}

int lsShell(char *argv[]) {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(".")) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.')
				fprintf(stdout, "%s\n", entry->d_name);
        }
        closedir(dir);
    } 
	else{
        errno = EIO;
        return 1;
    }
    fflush(stdout);
    return 0;
}

int echo( char * argv[]){
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
}