
int countSizeCom(command *com);

int countPipelineCom(pipelineseq * ln);

pipelineseq *parselineSafe(pipelineseq *ln, char *buf);

void findArgs(command *com, int sizeArgs, char **arguments);

int findDescriptors(command *com);

void createProc (int in, int out, char **arguments, command *com, pipeline *p);

void execPipeline (command *com, pipeline *p);

void execCommand(char *buf, char *end, int *readResult, struct stat *st, int *distanseAlreadyReaded, 
					int *isEndOfFile, pipelineseq *ln);