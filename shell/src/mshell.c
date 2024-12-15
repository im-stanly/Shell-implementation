#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#include "config.h"
#include "siparse.h"
#include "pipeLibery.h"
#include "execLibrary.h"
#include "bufforLibery.h"
#include "signalLibrary.h"

int main(int argc, char *argv[]){
	pipelineseq * ln;
	struct stat st;
	int distanseAlreadyReaded = 0, isEndOfFile = 0;

	setSignals();
	char buf[MAX_LINE_LENGTH + 1], *end;
	if(readingFromSTDIN(&st))
		checkWrite(write(1, PROMPT_STR, sizeof(PROMPT_STR)), sizeof(PROMPT_STR));
		
	int readResult = read(0, buf, MAX_LINE_LENGTH);
	while (readResult > 0){
		if(prepareBuf(buf, &end, &readResult, &distanseAlreadyReaded, &isEndOfFile, &st) != 0){
			readResult = 0;
			continue;
		}
		distanseAlreadyReaded = 2 + end - buf;
		readResult -= (distanseAlreadyReaded - 1);

		ln = parselineSafe(ln, buf);
		execCommand(buf, end, &readResult, &st, &distanseAlreadyReaded, &isEndOfFile, ln);
		readToBuf(buf, end, &readResult, &st, &distanseAlreadyReaded, &isEndOfFile);
	}
	if(readResult == -1){
		perror("read() was failed. Returns -1");
		exit(1);
	}

	return 0;
}