#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>

#include "config.h"
#include "siparse.h"
#include "pipeLibery.h"
#include "execLibrary.h"
#include "bufforLibery.h"
#include "signalLibrary.h"

int main(int argc, char *argv[]){
	pipelineseq * ln;
	struct stat st;
	int distanseAlreadyReaded = 0, isEndOfFile = 0, readResult = 0, prepareBufResult, debugg = 0;

	setSignals();
	char buf[MAX_LINE_LENGTH + 1], *end = NULL;

	while (readResult > 2 || isEndOfFile == 0){
		if(readingFromSTDIN(&st))
			checkWrite(write(1, PROMPT_STR, sizeof(PROMPT_STR)), sizeof(PROMPT_STR));

		prepareBufResult = prepareBuf(buf, &end, &readResult, &distanseAlreadyReaded, &isEndOfFile, &st, 0);
		if(prepareBufResult == 1){
			// printf("prepareBufResult == 1\n");
			continue;
		}
		else if(prepareBufResult == 2){
			// printf("prepareBufResult == 2\n");
			break;
		}
		if(readResult <= 2 && isEndOfFile == 1)
			break;

		ln = parselineSafe(ln, buf);
		execCommand(buf, end, &readResult, &st, &distanseAlreadyReaded, &isEndOfFile, ln);
	}
	if(readResult == -1){
		perror("read() was failed. Returns -1");
		exit(1);
	}

	return 0;
}