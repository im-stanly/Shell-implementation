void checkWrite(int res, long properSize);

int readingFromSTDIN(struct stat *st);

int prepareBuf(char *buf, char **end, int *readResult, int *isEndOfFile);