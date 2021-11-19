#ifndef TYPES_H
#define TYPES_H

/* Описание типов и базовая работа с ними. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define forever while (1)

#ifdef __cppcheck__
#define Expects(EXPR) [[expects: EXPR]]
#else
#define Expects(EXPR)
#endif

/* etoken -- Enum TOKEN */
typedef enum {
    END,       // Конец команды
    CMDNAME,   // Имя команды
    ARGLIST,   // Список аргументов
    SIMPLECMD, // Простая команда
    CONV,      // Конвеер
    CONVSEP,   // Конвеер (разделитель)
    REDIRI,    // Перенаправление ввода
    REDIRO,    // Перенаправление вывода, перезапись
    REDIROA,   // Перенаправление вывода, добавление
    REDIRIO,   // Перенаправление ввода/вывода
    CMD,       // Команда
    CONDCMD,   // Команда с условным выполнением
    CONDOK,    // Далее выполнение при успехе предыдущей команды
    CONDBAD,   // Далее выполнение при неудаче предыдущей команды
    BACKGRND,  // Выполнение в фоне
    DASHCMD    // Команда dash
} etoken;

/* Список токенов */
/* stoken -- Struct TOKEN */
typedef struct stoken {
    etoken tok;            // Сам токен
    struct stoken * ntok;  // next token
} token;
/******************/

/* Список лексем */
typedef struct _lexem {
    char* lex;
    struct _lexem * nlex;
} lexem;
/*****************/

typedef struct _cmdline {
    token* tokens; // Список токенов
    lexem* lexems; // Список лексем
} cmdline;

typedef struct _job_t {
    pid_t pid;
    char* name;
    struct _job_t * njob;
} job_t;

typedef enum {
    false,
    true
} bool;

void newlexem(lexem* p, char* nextlex) {
    lexem* newl = malloc(sizeof(struct _lexem));
    newl->nlex = p->nlex;
    newl->lex  = nextlex;
    p->nlex = newl;
}

void newtoken(token* p, etoken t) {
    token* newt = malloc(sizeof(struct stoken));
    newt->tok = t;
    newt->ntok = p->ntok;
    p->ntok = newt;
}

cmdline newcmdline(char* args) {
    cmdline cmd;
    
    cmd.tokens       = malloc(sizeof(token));
    cmd.tokens->ntok = malloc(sizeof(token));
    cmd.lexems       = malloc(sizeof(lexem));
    cmd.lexems->nlex = malloc(sizeof(lexem));
    cmd.lexems->lex  = malloc((strlen(args) + 1) * sizeof(char));
    
    cmd.tokens->tok       = DASHCMD;
    cmd.tokens->ntok->tok = END;
    cmd.tokens->ntok->ntok = NULL;
    strcpy(cmd.lexems->lex, args);
    cmd.lexems->nlex->lex = NULL;
    cmd.lexems->nlex->nlex = NULL;
    
    return cmd;
}

job_t* allocjob(void) {
    job_t* pj = malloc(sizeof(struct _job_t));
    pj->pid = -1;
    pj->name = NULL;
    pj->njob = NULL;
    return pj;
}

void delnjob(job_t* pj) {
    job_t* tj = pj->njob;
    pj->njob = pj->njob->njob;
    free(tj);
}

void freejobs(job_t* pj) {
    job_t* tj;
    if (pj == NULL) return;
    while (pj->njob != NULL) {
        tj = pj->njob;
        free(pj);
        pj = tj;
    }
    free(pj);
}

void print_token(etoken tok) {
    switch (tok) {
        case CMDNAME:   printf("CMDNAME");   break;
        case ARGLIST:   printf("ARGLIST");   break;
        case SIMPLECMD: printf("SIMPLECMD"); break;
        case CONV:      printf("CONV");      break;
        case CONVSEP:   printf("CONVSEP");   break;
        case REDIRI:    printf("REDIRI");    break;
        case REDIRO:    printf("REDIRO");    break;
        case REDIRIO:   printf("REDIRIO");   break;
        case REDIROA:   printf("REDIROA");   break;
        case CMD:       printf("CMD");       break;
        case CONDCMD:   printf("CONDCMD");   break;
        case CONDOK:    printf("CONDOK");    break;
        case CONDBAD:   printf("CONDBAD");   break;
        case BACKGRND:  printf("BACKGRND");  break;
        case DASHCMD:   printf("DASHCMD");   break;
        case END:       printf("END");       break;
        default:        printf("???");       break;
    }
}

void print_cmdline(cmdline cmd) {
    lexem* plex = cmd.lexems;
    token* ptok = cmd.tokens;
    while (ptok->tok != END) {
        print_token(ptok->tok);
        printf(" \t\t[%s]\n", plex->lex);
        plex = plex->nlex;
        ptok = ptok->ntok;
    }
    printf("END\n\n");
}

void freecmd(cmdline cmd) {
    lexem *plex = cmd.lexems, *tl;
    token *ptok = cmd.tokens, *tt;
    
    free(plex->lex);
    
    while (ptok != NULL) {
        tt = ptok->ntok;
        free(ptok);
        ptok = tt;
    }
    while (plex != NULL) {
        tl = plex->nlex;
        free(plex);
        plex = tl;
    }
}

int toklen(token* ptok){
    int n = 0;
    while (ptok != NULL) {
        ptok = ptok->ntok;
        n++;
    }
    return n;
}

etoken* get_array_of_tokens(cmdline cmd) {
    token* ptok = cmd.tokens;
    int n = toklen(ptok);
    
    etoken* ret = malloc(n * sizeof(token));
    
    for (int i = 0; i < n; i++) {
        ret[i] = ptok->tok;
        ptok = ptok->ntok;
    }
    
    return ret;
}

#endif
