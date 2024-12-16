#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>

#include "config.h"
#include "siparse.h"
#include "pipeLibery.h"
#include "bufforLibery.h"
#include "signalLibrary.h"

int main(int argc, char *argv[]){
	pipelineseq * ln;
	struct stat st;
	int distanseAlreadyReaded = 0, isEndOfFile = 0, readResult = 0, prepareBufResult;

	setSignals();
	char buf[MAX_LINE_LENGTH + 1], *end = NULL;

	while (readResult > 2 || isEndOfFile == 0){
		if(readingFromSTDIN(&st))
			checkWrite(write(1, PROMPT_STR, sizeof(PROMPT_STR)), sizeof(PROMPT_STR));

		prepareBufResult = prepareBuf(buf, &end, &readResult, &isEndOfFile);
		if(prepareBufResult == 1)
			continue;
		else if(prepareBufResult == 2)
			break;

		if(readResult <= 2 && isEndOfFile == 1)
			break;

		ln = parselineSafe(ln, buf);
		execCommand(buf, ln);
	}
	if(readResult == -1){
		perror("read() was failed. Returns -1");
		exit(1);
	}

	return 0;
}