#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "config.h"
#include "siparse.h"
#include "pipeLibery.h"
#include "signalLibrary.h"

/*
 * 1. Pamietac o '#', '\n' oraz ' ' bo to bedzie przesuniecie o 1 pozucje albo że komentarz(lub \n) czyli wczytujemy cala linie do konca
 
 * 2. Pamietac o wypisaniu PROMPT_STR czyli jakies:
		int isReadingFromSTDIN = readingFromSTDIN(st);
		if(st != NULL && isReadingFromSTDIN && (*readResult) < 2)
			checkWrite(write(1, PROMPT_STR, sizeof(PROMPT_STR)), sizeof(PROMPT_STR));

 * 3. Pamietac ze jak za dluga linia to:
		if((*readResult) == MAX_LINE_LENGTH){
			fprintf(stderr, "Syntax error.\n");
			fflush(stderr);
		}
 */

/*
	Documentation:

	- prepareBuf() zwraca:
		1 - jeśli trzeba powtorzyc czytanie czyli w main zrobic continue
		2 - jeśli read() zwróciło 0 czyli koniec pliku
		0 - jeśli wszystko poszło dobrze
*/

void checkWrite(int res, long properSize){
	printBeforePrompt();
	if(res == -1){
		perror("write() was failed: returns -1");
		exit(1);
	}
	else if(res != properSize){
		perror("write() was failed: returns invalid size");
		exit(1);
	}
}

int readingFromSTDIN(struct stat *st){
	if(fstat(0, st)){ 
		perror("fstat() function was failed");
		exit(1);
    }
	return S_ISCHR(st->st_mode);
}

char *splitLine(char *buf, int bufSize){
	char *end = memchr(buf, '\n', bufSize);

	if(end == NULL){
		end = memchr(buf, '\0', bufSize);
		if(end == NULL)
			return NULL;
	}
	end[0] = '\0';
	return end;
}

void handle_long_line(char *buf, int *r, char *curr_line) {
	fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
	while(1) {
		memmove(buf, curr_line, 0);
		curr_line = buf;
		*r = read(STDIN_FILENO, curr_line, MAX_LINE_LENGTH);
		if (*r == -1) {
			perror("read() failed");
			exit(EXIT_FAILURE);
		}
		if(*r == 0)
			exit(EXIT_SUCCESS);
		if(strchr(curr_line, '\n') != NULL)
			break;
	}
}

void process_input(char *buf, int r, int isTerminal) {
	buf[r] = '\0';
	char *curr_line = buf, *next_line=NULL;
	int new_r;

	while(r > 0) {
		next_line = strchr(curr_line,'\n');
		if(next_line != NULL) {
			next_line[0] = '\0';
			if(next_line - curr_line > 0 && curr_line[0] != '#')
				execCommand(curr_line);
			r -= next_line - curr_line + 1;
			curr_line = next_line + 1;
		} else {
			if(r >= MAX_LINE_LENGTH){
				handle_long_line(buf, &r, curr_line);
				next_line = strchr(curr_line,'\n');
				next_line[0] = '\0';
				r -= next_line - curr_line + 1;
				curr_line = next_line + 1;
				continue;
			}
			memmove(buf, curr_line, r);
			curr_line = buf;
			new_r = read(STDIN_FILENO, curr_line + r, MAX_LINE_LENGTH - r);
			if(new_r == -1) {
				perror("read() failed");
				exit(EXIT_FAILURE);
			}
			if(new_r == 0)
				exit(EXIT_SUCCESS);
			r += new_r;
			buf[r] = '\0';
		}
	}
}

void input_handler(int isTerminal) {
	char buf[MAX_LINE_LENGTH];
	int r;
	do {
		if(isTerminal) {
			if(write(STDOUT_FILENO, PROMPT_STR, strlen(PROMPT_STR)) == -1) {
				perror("write() failed");
				exit(EXIT_FAILURE);
			}
		}
		r = read(STDIN_FILENO, buf, MAX_LINE_LENGTH);
		if(r == -1 && errno == EINTR)
			continue;
		if(r == -1) {
			perror("read() failed");
			exit(EXIT_FAILURE);
		}
		if(r == 0)
			break;
		process_input(buf, r, isTerminal);
	} while(1);
}