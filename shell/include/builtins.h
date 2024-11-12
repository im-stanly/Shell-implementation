#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2

typedef struct {
	char* name;
	int (*fun)(char**); 
} builtin_pair;

extern builtin_pair builtins_table[];
int exitShell( char * argv[]);
int cdShell(char *argv[]);
int killShell(char *argv[]);
int lsShell(char *argv[]);
int echo( char * argv[]);
#endif /* !_BUILTINS_H_ */
