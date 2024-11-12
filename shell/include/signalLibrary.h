#ifndef TMP_H
#define TMP_H

typedef struct proc{
	int pid;
	int statusPid;
} proc;

typedef struct proc_count{
	proc arr[2048];
    int index;
} proc_count;

extern proc_count backgroundProc;
extern proc_count foregroundProc;

void addPid(int npid, int isFG);

int searchID(int curpid, int isFG);

void setStatus(int pid, int statusPid, int isFG);

void deletePid(int pid, int isFG);

void printAll(int isFG);

void clearAll(int isFG);

void sc_handler(int tmp);

void printBeforePrompt();

void setSignals();

#endif