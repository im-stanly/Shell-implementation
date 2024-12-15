#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "config.h"
#include "siparse.h"
#include "pipeLibery.h"
#include "signalLibrary.h"

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

char *splitLine(char *buf, int bufSize, int *isEndOfFile){
	char *end = memchr(buf, '\n', bufSize);

	if(end == NULL){
		end = memchr(buf, '\0', bufSize);
		if(end != NULL)
			*isEndOfFile = 1;
		return end != NULL ? end : NULL;
	}
	end[0] = '\0';
	return end;
}

void lineFullRead(char *buf, int *readResult, int *distanseAlreadyReaded, char *end){
	char *endLine = memchr(buf, '\n', (*readResult));
	if(endLine == NULL)
		endLine = memchr(buf, '\0', (*readResult));
	while (endLine == NULL && (*readResult) < MAX_LINE_LENGTH){
		*readResult += read(0, (buf + (*readResult)), (MAX_LINE_LENGTH - (*readResult)));
		*distanseAlreadyReaded = 0;
		end = buf - 1;
		endLine = memchr(buf, '\n', (*readResult));
		if(endLine == NULL)
			endLine = memchr(buf, '\0', (*readResult));
	}
}

int isBufforOkay(char *buf, int *readResult, int *distanseAlreadyReaded, char *end){
	char tmp = buf[0];
	lineFullRead(buf, readResult, distanseAlreadyReaded, end);

	if(tmp == '\n' || ((*readResult) > 2 && tmp == '\0') || tmp == '#' || tmp == ' ')
		return 0;
	if(tmp != '\0' && tmp != EOF)
		return 1;
	return 0;
}

void readToBuf(char *buf, char *end, int *readResult, struct stat *st, int *distanseAlreadyReaded, int *isEndOfFile){
	int isReadingFromSTDIN = readingFromSTDIN(st);
	if(st != NULL && isReadingFromSTDIN && (*readResult) < 2)
		checkWrite(write(1, PROMPT_STR, sizeof(PROMPT_STR)), sizeof(PROMPT_STR));
	
	memmove(buf, end + 1, (*readResult));
	if((isReadingFromSTDIN && (*readResult) < 2) || (!isReadingFromSTDIN && (*isEndOfFile) == 0)){
		*readResult += read(0, (buf + (*readResult)), (MAX_LINE_LENGTH - (*readResult)));
		*distanseAlreadyReaded = 0;
		end = buf - 1;
	}
}

int checkBufBegin(char *buf, char *end, int *readResult, int *distanseAlreadyReaded, int *isEndOfFile, struct stat *st){
	if(!isBufforOkay(buf, readResult, distanseAlreadyReaded, end)){
		if(buf[0] == '\0' || buf[0] == EOF){
			(*isEndOfFile) = 1;
			return 1;
		}
		if(buf[0] == '\n' || buf[0] == ' '){
			if((*readResult) > 1)
				(*readResult)--;
			else
				(*readResult) = 0;
			end = buf;
			(*distanseAlreadyReaded) = 2 + end - buf;
			readToBuf(buf, end, readResult, st, distanseAlreadyReaded, isEndOfFile);
		}
		if(buf[0] == '#'){
            char *newline = strchr(buf, '\n');
            if(newline){
                (*distanseAlreadyReaded) = 2 + newline - buf;
				(*readResult) -= (*distanseAlreadyReaded) - 1;
				readToBuf(buf, newline, readResult, st, distanseAlreadyReaded, isEndOfFile);
            }
			else{
				(*distanseAlreadyReaded) = 0;
				(*readResult) = 0;
			}
        }
		return -1;
	}
	return 0;
}

void handleEndOfLine(char *buf, char **end, int *readResult, int *distanseAlreadyReaded, int *isEndOfFile, struct stat *st, int *result){
	int endlFinded = 0;
	char *endLine;
	
	if((*readResult) == MAX_LINE_LENGTH){
		fprintf(stderr, "Syntax error.\n");
		fflush(stderr);
	}
	*result = -1;
	(*readResult) = read(0, buf, MAX_LINE_LENGTH);

	while (endlFinded == 0 && (*readResult) > 0){
		endLine = splitLine(buf, (*readResult), isEndOfFile);
		if(endLine != NULL){
			endlFinded = 1;
			(*distanseAlreadyReaded) = 2 + endLine - buf;
			(*readResult) -= ((*distanseAlreadyReaded) - 1);
			readToBuf(buf, endLine, readResult, st, distanseAlreadyReaded, isEndOfFile);
			*end = endLine;
			break;
		}
		else if(*isEndOfFile == 0){
			(*readResult) = read(0, buf, MAX_LINE_LENGTH);
		}
	}
	(*distanseAlreadyReaded) = 0;
}

int prepareBuf(char *buf, char **end, int *readResult, int *distanseAlreadyReaded, int *isEndOfFile, struct stat *st){
	int result = -1;

	while (result != 0 && (*readResult) > 0){
		result = checkBufBegin(buf, *end, readResult, distanseAlreadyReaded, isEndOfFile, st);

		if(result == 1 || (*readResult) == 0)
			return 1;
		if(result == 0){
			*end = splitLine(buf, (*readResult), isEndOfFile);
			if(*isEndOfFile == 1)
				return 1;
			if((*end) == NULL){
				handleEndOfLine(buf, end, readResult, distanseAlreadyReaded, isEndOfFile, st, &result);
				continue;
			}
			result = 0;
		}
	}
	return (*readResult) > 0 ? 0 : 1;
}