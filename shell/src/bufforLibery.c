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

/**
 * discardLine(...) will discard the current line from buf, reading more if necessary until a newline or EOF.
 * After completion, buf will contain only data after the discarded line (if any),
 * and readResult will be updated accordingly.
 */
void discardLine(char *buf, int *readResult) {
    char *newline = strchr(buf, '\n');
    if (newline) {
        int lineLen = (int)(newline - buf) + 1;
        *readResult -= lineLen;
        memmove(buf, newline + 1, *readResult);
        buf[*readResult] = '\0';
        return;
    }

    while (1) {
        *readResult = 0;
        buf[0] = '\0';

        int n = read(STDIN_FILENO, buf, MAX_LINE_LENGTH);
        if (n == -1 && errno == EINTR) {
            continue;
        } else if (n == -1) {
            perror("read() failed");
            exit(1);
        } else if (n == 0) {
            *readResult = 0;
            buf[0] = '\0';
            return;
        }

        *readResult = n;
        buf[*readResult] = '\0';

        newline = strchr(buf, '\n');
        if (newline) {
            int lineLen = (int)(newline - buf) + 1;
            *readResult -= lineLen;
            memmove(buf, newline + 1, *readResult);
            buf[*readResult] = '\0';
            return;
        }
    }
}

/**
 * prepareBuf(...) prepares the buffer and read a line.
 * 1. If the first char is ' ' or '\n', skip it.
 * 2. If the first char is '#', discard the whole line (comment).
 * 3. If no newline is found before reaching MAX_LINE_LENGTH, print error, discard the whole line.
 * 4. Otherwise, find '\n' and replace it with '\0'.
 *
 * Returns:
 * - 0 if a line has been successfully read into buf
 * - 1 if we skipped something and should try reading again
 * - 2 if EOF is reached with no more data
 */
int prepareBuf(char *buf, char **end, int *readResult, int *isEndOfFile) {
    if (*end != NULL) {
        int lineLen = (int)(*end - buf) + 1;
        (*readResult) -= lineLen;
        memmove(buf, *end + 1, *readResult);
        buf[*readResult] = '\0';
        *end = NULL;
    }

    while (1) {
        if (!*isEndOfFile && *readResult < MAX_LINE_LENGTH) {
            int n = read(STDIN_FILENO, buf + (*readResult), MAX_LINE_LENGTH - (*readResult));
            if (n == -1 && errno == EINTR) {
                continue;
            } else if (n == -1) {
                perror("read() failed");
                exit(1);
            } else if (n == 0) {
                *isEndOfFile = 1;
            } else {
                (*readResult) += n;
                buf[*readResult] = '\0';
            }
        }

        char *newlinePos = strchr(buf, '\n');
        if (newlinePos)
            break;

        if (*isEndOfFile)
            break;

        if (*readResult == MAX_LINE_LENGTH) {
            fprintf(stderr, "Syntax error.\n");
            fflush(stderr);
            discardLine(buf, readResult);
            return 1;
        }

    }

    while (*readResult > 0 && (buf[0] == ' ' || buf[0] == '\n')) {
        (*readResult)--;
        memmove(buf, buf + 1, *readResult);
        buf[*readResult] = '\0';
        return 1;
    }

    if (*readResult == 0 && *isEndOfFile)
        return 2;

    if (*readResult == 0)
        return 1;

    if (buf[0] == '#') {
        discardLine(buf, readResult);
        return 1;
    }

    char *newline = splitLine(buf, (*readResult) + 1);
    if (newline)
        *end = newline;
    else 
        *end = NULL; 

    return 0;
}