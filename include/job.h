#ifndef JOB_H
#define JOB_H

/* Управление фоновыми процессами */

#include <signal.h>
#include "types.h"

job_t* jobs = NULL;

void initjobs   (void);
int  countjobs  (void);
int  listjobs   (void);
void newjob     (char* name, pid_t pid);
int  continuejob(int n);

void initjobs(void) {
    jobs = allocjob();
}

void newjob(char* name, pid_t pid) {
    job_t* pj = jobs;
    
    while (pj->njob != NULL) pj = pj->njob;

    pj->njob = allocjob();
    pj->njob->pid  = pid;
    pj->njob->name = malloc((strlen(name) + 1) * sizeof(char));
    strcpy(pj->njob->name, name);
    pj->njob->njob = NULL;
    printf("[+] Имя: %s\t\tPID: %u\n", name, pid);
}

int countjobs(void) {
    job_t* pj = jobs->njob;
    int ret = 0;
    while (pj != NULL) {
        pj = pj->njob;
        ret++;
    }
    return ret;
}

int listjobs(void) {
    job_t* pj = jobs->njob;
    int n = 1;
    while (pj != NULL) {
        printf("[%d] Имя: %s\t\tPID: %u\n", n++, pj->name, pj->pid);
        pj = pj->njob;
    }
    return 0;
}

int continuejob(int n) {
    int ret = 0, st;
    int all = countjobs();
    if ((n <= 0) || (n > all)) {
        printf("Недопустимый номер задачи -- %d (всего %d)\n", n, all);
        return 1;
    }
    job_t* pj = jobs;
    while (--n) pj = pj->njob;
    pid_t pid = pj->njob->pid;
    char* name = pj->njob->name;
    kill(pid, SIGCONT);
    printf("Задача %s (PID: %u) продолжена.\n", name, pid);
    delnjob(pj);
    ret = waitpid(pid, &st, 0);
    if (ret == -1) perror("wait");
    else if (WIFEXITED(st)) ret = WEXITSTATUS(st);
    else if (WIFSIGNALED(st)) ret = -WTERMSIG(st);
    else if (WIFSTOPPED(st)) {
        printf("[!] Процесс остановлен. PID = %u\n", pid);
        newjob(name, pid);
    } else panic(UNKNOWN, "continuejob не смог определить судьбу процесса.");
    
    return ret;
}

#endif
