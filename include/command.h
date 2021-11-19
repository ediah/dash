#ifndef COMMAND_H
#define COMMAND_H

/* Команды шелла. */

#include "job.h"

int  dash_ret   (char** args);
int  dash_jobs  (char** args);
int  execdashcmd(char** args);
char isdashcmd  (char*  name);

int dash_ret(char** args) {
    int ret = 0;
    if (args[1] == NULL) {
        if (scanf("%d", &ret) == 0)
            panic(FLAGS, "предоставьте число.");
    } else {
        if (sscanf(args[1], "%d", &ret) == 0)
            panic(FLAGS, "предоставьте число.");
    }
    return ret;
}

int dash_jobs(char** args) {
    int ret = 0;
    if (args[1] == NULL)
        panic(FLAGS, "введите команду.");
    if      (strcmp(args[1], "count") == 0) ret = countjobs();
    else if (strcmp(args[1], "list")  == 0) ret = listjobs();
    else if (strcmp(args[1], "cont")  == 0) {
        int n;
        sscanf(args[2], "%d", &n);
        ret = continuejob(n);
    } else if (strcmp(args[1], "help")  == 0) {
        printf("Доступные команды:\n");
        printf("\tcount    -- количество запущенных задач\n");
        printf("\tlist     -- предоставить список задач\n");
        printf("\tcont <n> -- продолжить задачу с номером n\n");
    } else {
        ret = 1;
        printf("Неизвестная команда -- «%s».\n", args[1]);
        printf("По команде «jobs help» можно получить дополнительную информацию.\n");
    }
    return ret;
}

int dash_murgo(char** args) {
    int ret = 0;
    int pr = countjobs() % 2;
    char* face[2] = {"'w'", "^o^"};
    printf(".     .\n|\\___/|\n|=%s=| mur~~\n(\")_(\")\n\n", face[pr]);
    return ret;
}

char isdashcmd(char* name) {
    char* commands[4] = {"ret", "jobs", "murgo", NULL};
    for (int i = 0; commands[i] != NULL; i++) {
        if (strcmp(commands[i], name) == 0) return 1;
    }
    return 0;
}

int execdashcmd(char** args) {
    int ret;
    if      (strcmp(args[0], "ret"  ) == 0) ret = dash_ret  (args);
    else if (strcmp(args[0], "jobs" ) == 0) ret = dash_jobs (args);
    else if (strcmp(args[0], "murgo") == 0) ret = dash_murgo(args);
    else panic(UNKNOWN, "неизвестная команда dash!");
    return ret;
}

#endif
