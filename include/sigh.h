#ifndef SIGH_H
#define SIGH_H

#include <signal.h>
#include "job.h"

// Обработка сигналов.

void checkjobs(int sig) {
    int ret, st;
    job_t *pj = jobs;
    while (pj != NULL) {
        if (pj->njob == NULL) break;
        ret = waitpid(pj->njob->pid, &st, WNOHANG);
        if (ret == -1) {
            #ifdef verbose
            perror("waitpid");
            #endif
        } else if (ret != 0) {
            #ifdef verbose
            printf("Процесс %s (PID: %u) был завершён с кодом %d.\n", 
                                     pj->njob->name, pj->njob->pid, st);
            #endif
            delnjob(pj);
        }
        pj = pj->njob;
    }
    
}

void sigign(void) {
    signal(SIGINT,  SIG_IGN);
    signal(SIGCHLD, checkjobs);
    signal(SIGTSTP, SIG_IGN);
}

void sigdfl(void) {
    signal(SIGINT,  SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
}

#endif
