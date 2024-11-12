void checkWrite(int res, long properSize);

int readingFromSTDIN(struct stat *st);

char *splitLine(char *buf, int bufSize, int *isEndOfFile);

int isBufforOkay(char *buf, int *readResult, int *distanseAlreadyReaded, char *end);

void readToBuf(char *buf, char *end, int *readResult, struct stat *st, int *distanseAlreadyReaded, int *isEndOfFile);

int checkBufBegin(char *buf, char *end, int *readResult, int *distanseAlreadyReaded, int *isEndOfFile, struct stat *st);

void printBuf(char *buf);

int prepareBuf(char *buf, char **end, int *readResult, int *distanseAlreadyReaded, int *isEndOfFile, struct stat *st);