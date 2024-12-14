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
	// pipelineseq * ln;
	struct stat st;
	// int distanseAlreadyReaded = 0, isEndOfFile = 0, readResult = 0, prepareBufResult, debugg = 0;

	setSignals();
	
	if(fstat(0, &st) == -1) {
		perror("fstat() failed");
		exit(EXIT_FAILURE);
	}
	int isTerminal = S_ISCHR(st.st_mode);
	input_handler(isTerminal);

	return 0;
}