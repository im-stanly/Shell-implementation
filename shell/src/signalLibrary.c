#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "signalLibrary.h"

proc_count backgroundProc;
proc_count foregroundProc;

void addPid(int npid, int isFG){
    if(isFG == 1){
        if(foregroundProc.index >= 2048)
            exit(1);
        foregroundProc.arr[foregroundProc.index].pid = npid;
        foregroundProc.index = foregroundProc.index + 1;
    }
    else{
        if(backgroundProc.index >= 2048)
            exit(1);
        backgroundProc.arr[backgroundProc.index].pid = npid;
        backgroundProc.index = backgroundProc.index + 1;
    }
}

int searchID(int curpid, int isFG){
    if(isFG == 1){
        for (int i = 0; i < foregroundProc.index; i++){
            if(foregroundProc.arr[i].pid == curpid)
                return i;
        }
        return -1;
        
    }
    else{
        for (int i = 0; i < backgroundProc.index; i++){
            if(backgroundProc.arr[i].pid == curpid)
                return i;
        }
        return -1;
    }
}

void setStatus(int pid, int statusPid, int isFG){
    int id = searchID(pid, isFG);
    if(id == -1)
        exit(1);
    if(isFG == 1)
        foregroundProc.arr[id].statusPid = statusPid;
    else
        backgroundProc.arr[id].statusPid = statusPid;
}

void deletePid(int pid, int isFG){
    int id = searchID(pid, isFG);
    if(id == -1)
        exit(1);

    if(isFG == 1){
        for (int i = id + 1; i < foregroundProc.index; i++)
            foregroundProc.arr[i - 1] = foregroundProc.arr[i];
        
        foregroundProc.index = foregroundProc.index - 1;
    }
    else{
        for (int i = id + 1; i < backgroundProc.index; i++)
            backgroundProc.arr[i - 1] = backgroundProc.arr[i];

        backgroundProc.index = backgroundProc.index - 1;
    }
}

void printAll(int isFG){
    if(isFG == 1){
        for (int i = 0; i < foregroundProc.index; i++)
            fprintf(stdout, "%d\n", foregroundProc.arr[i].pid);
    }
    else{
        for (int i = 0; i < backgroundProc.index; i++)
            fprintf(stdout, "%d\n", backgroundProc.arr[i].pid);
    }
}

void clearAll(int isFG){
    if(isFG == 1)
        foregroundProc.index = 0;
    else
        backgroundProc.index = 0;
}

void sc_handler(int tmp){ 
    while(1){ 
        int sigResult;
        int pid = waitpid(-1, &sigResult , WNOHANG);

        if (pid <= 0 || errno == ECHILD)
            break; 

        if(searchID(pid, 1) >= 0)
            deletePid(pid, 1);
        else{
            addPid(pid, 0);
            setStatus(pid, sigResult, 0);
        }
    }
}

void printBeforePrompt(){
    for (int i = 0; i < backgroundProc.index; i++){
        fprintf(stdout, "Background process %d terminated.", backgroundProc.arr[i].pid);
        if(WIFEXITED(backgroundProc.arr[i].statusPid)){
            fprintf(stdout, " (exited with status (%d))", WEXITSTATUS(backgroundProc.arr[i].statusPid));
        }
        else if(WIFSIGNALED(backgroundProc.arr[i].statusPid)){
            fprintf(stdout, " (killed by signal (%d))", WTERMSIG(backgroundProc.arr[i].statusPid));
        }

        printf("\n");
        fflush(stdout);
    }
    clearAll(0);
}

void setSignals(){
	foregroundProc.index = 0;
	backgroundProc.index = 0;
    
	struct sigaction sigAct = {
		.sa_handler = SIG_IGN,
		.sa_flags = 0
	};
	sigemptyset(&sigAct.sa_mask);
	sigaction(SIGINT, &sigAct, NULL);

	struct sigaction sigActForCHIL = { 
		.sa_handler = sc_handler,
		.sa_flags = SA_NOCLDSTOP
	};
	sigemptyset(&sigActForCHIL.sa_mask);
	sigaction(SIGCHLD, &sigActForCHIL, NULL);
}