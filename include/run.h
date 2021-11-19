#ifndef RUN_H
#define RUN_H

/* Обработчик командной строки. */

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "types.h" 
#include "token.h" 
#include "command.h"
#include "job.h"
#include "sigh.h"

//#define verbose
#define MAX_LEN 100
#define EXIT_CMD "q"

bool conv = false;

void   job       (FILE* stream);
char** parse_args(char* name, char* args);
void   prompt    (char * cmd, FILE* stream);
void   exec      (cmdline cmd, bool bckgr);
int    run       (char *name, char *args, int fdin, int fdout);
int    background(char *name, char *args, int fdin, int fdout);
void   safe_exit(int code);

char** parse_args(char* name, char* args) {
    char** ret = malloc(sizeof(char*) * 2), **rt;
    while (name[0] == ' ') name = &(name[1]);
    ret[0] = name;
    ret[1] = NULL;
    
    if (args == NULL) return ret;
    
    while (args[0] == ' ') args = &(args[1]);
    char* start = NULL;
    
    
    int i = 0, j = 1;
    
    for (i = 0; args[i] != '\0'; i++);
    for (i = i - 1; args[i] == ' '; i--);
    args[i+1] = '\0';
    
    i = 0;
    forever {
        if (((args[i] == ' ') || (args[i] == '\0')) && (start != NULL)) {
            rt = realloc(ret, sizeof(char*) * (j + 2));
            if (rt == NULL) {
                free(ret);
                panic(UNKNOWN, "не удалось перевыделить память.");
            }
            ret = rt;
            ret[j] = start;
            ret[++j] = NULL;
            start = NULL; 
            if (args[i] == '\0') break;
            else args[i++] = '\0';
        }
        if ((args[i] != ' ') && (start == NULL)) start = &(args[i]);
        i++;
    }
    
    return ret;
}

int run(char *name, char *args, int fdin, int fdout) {
       
    char** vargs = parse_args(name, args);
    
    //for (int i = 0; vargs[i] != NULL; i++) printf("%s ", vargs[i]);
    //printf("\n");
    
    int st, ret = 0;
    
    if (isdashcmd(vargs[0])) {
        if (fdin != -1)  dup2(fdin,  0);
        if (fdout != -1) dup2(fdout, 1);
        ret = execdashcmd(vargs);
    } else {
        pid_t pid;
        switch (pid = fork()) {
            case -1:
                perror("run: fork");
                panic(UNKNOWN, "не удалось создать сыновий процесс.");
                break;
            case 0:
                sigdfl();
                if (fdin != -1)  dup2(fdin,  0);
                if (fdout != -1) dup2(fdout, 1);
                
                execvp(vargs[0], vargs);
                break;
            default:
                if (conv) break;
                #ifdef debug
                printf("PPID %u создал PID %u\n", getpid(), pid);
                #endif
                ret = waitpid(pid, &st, WUNTRACED);
                if (ret == -1) perror("wait");
                else if (WIFEXITED(st)) ret = WEXITSTATUS(st);
                else if (WIFSIGNALED(st)) ret = -WTERMSIG(st);
                else if (WIFSTOPPED(st)) {
                    printf("[!] Процесс остановлен. PID = %u\n", pid);
                    newjob(name, pid);
                } else panic(UNKNOWN, "run не смог определить судьбу процесса.");
        }
    }
    #ifdef verbose
    if (ret < 0)
        printf("[С%d] %s\n", -ret, name); // Сигнал
    else
        printf("[В%d] %s\n", ret, name);  // Возврат
    #endif
    
    if (fdin  != -1) close(fdin);
    if (fdout != -1) close(fdout);
    free(vargs);
    return ret;
}

int background(char *name, char *args, int fdin, int fdout) {
    pid_t pid;
    char** vargs = parse_args(name, args);
    switch(pid = fork()) {
        case 0:
            if (fdin == -1) fdin = open("/dev/null", O_RDONLY);
            if (fdout == -1) fdout = open("/dev/null", O_WRONLY);
            dup2(fdin,  0);
            dup2(fdout, 1);
            
            execvp(vargs[0], vargs);
            exit(0);
            break;
        default:
            newjob(vargs[0], pid);
            break;
    }
    return 0;
}

void exec(cmdline cmd, bool bckgr) {
    mode_t mo = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fdin = -1, fdout = -1;
    int npipe[2];
    
    int ret = 0;
    lexem* plex = cmd.lexems;
    token* ptok = cmd.tokens;

    char *name = NULL, *args = NULL;
    
    forever {
        //print_token(ptok->tok);
        //printf("\t%s\n", plex->lex);
        
        if ((ptok->tok == CMDNAME) || (ptok->tok == END)) {
            if (name == NULL) name = plex->lex;
            else if (name[0] != '\0') {
                if (!bckgr)
                    run(name, args, fdin, fdout);
                else
                    background(name, args, fdin, fdout);
                
                if (ptok->tok == END) break;
                
                name = plex->lex;
                args = NULL;

                fdin = -1;
                fdout = -1;
            }
        }
        
        if (ptok->tok == ARGLIST) args = plex->lex;
        
        if ((ptok->tok == REDIRI) && (fdin == -1))
            fdin = open(plex->lex, O_RDONLY);
        if ((ptok->tok == REDIRO) && (fdout == -1))
            fdout = open(plex->lex, O_WRONLY | O_CREAT, mo);
        if ((ptok->tok == REDIROA) && (fdout == -1))
            fdout = open(plex->lex, O_WRONLY | O_CREAT | O_APPEND, mo);
        
        if (ptok->tok == CONVSEP) {
            pipe(npipe);
            fdout = npipe[1];
            conv = true;
            
            if (!bckgr)
                run(name, args, fdin, fdout); // Запуск
            else
                background(name, args, fdin, fdout);
            name = NULL;
            args = NULL;
            fdin = npipe[0];
            fdout = -1;
            conv = false;
        }
        
        if ((ptok->tok == CONDOK) || (ptok->tok == CONDBAD)) {
            if (name != NULL) {
                if (!bckgr)
                    ret = run(name, args, fdin, fdout); // Запуск
                else
                    ret = background(name, args, fdin, fdout);
            } else ret = 0;
            if ((ret == 0) && (ptok->tok == CONDBAD)) break;
            if ((ret != 0) && (ptok->tok == CONDOK)) break;
            name = NULL;
            args = NULL;
            fdin = -1;
            fdout = -1;
        }
        
        if (ptok->tok == DASHCMD) {
            lexem* jmplex = plex->nlex;
            token* jmptok = ptok->ntok;
            cmdline c = tokenize(plex->lex);
            exec( c, false );
            plex = jmplex;
            ptok = jmptok;
            freecmd(c);
        }
        
        if (ptok->tok == BACKGRND) {
            cmdline c = tokenize(plex->nlex->lex);
            #ifdef debug
            print_cmdline(c);
            #endif
            exec(c, true);
            ptok = ptok->ntok;
            plex = plex->nlex;
            freecmd(c);
        }
        
        ptok = ptok->ntok;
        plex = plex->nlex;
        if (ptok == NULL) break;
    }
}

void job(FILE* stream) {
    char command[MAX_LEN];
    cmdline cmd;
    prompt(command, stream);
    if (command[0] == '\0') return;
    if (strcmp(command, EXIT_CMD) != 0) {
        cmd = tokenize(command);
        #ifdef debug
        print_cmdline(cmd);
        #endif
        exec(cmd, false);
        freecmd(cmd);
    } else {
        printf("Выход...\n");
        freejobs(jobs);
        fclose(stream);
        exit(0);
    }
}

void prompt(char *cmd, FILE* stream) {
    int i = 0;
    if (stream == stdin) printf("dash> ");
    while ((cmd[i++] = getc(stream)) != '\n') 
        if (i > MAX_LEN) panic(RUN, "слишком большая команда.");
        else if (cmd[i-1] == EOF) {
            freejobs(jobs);
            fclose(stream);
            exit(0);
        }
    cmd[i-1] = '\0';
}

void safe_exit(int code) {
    
    exit(code);
}

#endif
