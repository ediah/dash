#ifndef TOKEN_H
#define TOKEN_H

/* Обработка грамматики и разбиение на токены с лексемами. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h"
#include "panic.h"

cmdline tokenize(char* args);                  // Токенайзер
int parse_dashcmd  (lexem* plex, token* ptok); // Команда Shella
int parse_condcmd  (lexem* plex, token* ptok); // С условным выполнением
int parse_cmd      (lexem* plex, token* ptok); // Команда
int parse_conv     (lexem* plex, token* ptok); // Конвейер
int parse_simplecmd(lexem* plex, token* ptok); // Простая команда
int parse_redirio  (lexem* plex, token* ptok); // Перенаправление

int parse_redirio(lexem* plex, token* ptok){
    int ret = 0, i = 0, in = 0, out = 0;
    
    while ( (plex->lex[i] != '>') && (plex->lex[i] != '<') ) {
        if (plex->lex[i] == '\0') return 0; // ничего не нашли
        i++;
    }
    
    if (plex->lex[i] == '>') {
        out = 1;
        plex->lex[i] = '\0';
        
        if (plex->lex[i+1] == '>') {
            ptok->tok = REDIROA;
            i++; // append
        } else ptok->tok = REDIRO;
        
    } else if (plex->lex[i] == '<') {
        in = 1;
        plex->lex[i] = '\0';
        
        ptok->tok = REDIRI;
    }
    
    while (plex->lex[++i] == ' '); // возможные лишние пробелы
    plex->lex = &(plex->lex[i]); // тут начинается имя файла
    for (i = 0; (plex->lex[i] != ' ') && (plex->lex[i] != '\0'); i++);
    
    if (plex->lex[i] != '\0') {
        plex->lex[i++] = '\0';         // тут оно закончилось
        
        while (plex->lex[i] == ' ') i++;
        
        if (plex->lex[i] != '\0') {
            
            newlexem(plex, &(plex->lex[i]));
            newtoken(ptok, REDIRIO);
            
            ret = parse_redirio(plex->nlex, ptok->ntok); // 0-none, 1-in, 2-out
        }
    }
    
    
    if ( ((ret == 1) && in) || ((ret == 2) && out) )
      panic(SYNTAX, "недопустимое количество входных/выходных файлов.");
    
    return in ? 1 : 2; // тут в любом случае либо in, либо out будет
}

int parse_simplecmd(lexem* plex, token* ptok) {
    int i, ret = 0;
    for (i = 0; (plex->lex[i] == ' '); i++);
    //plex->lex = &(plex->lex[i]);
    
    bool wascmd = false;
    for (; (plex->lex[i] != ' ') && (plex->lex[i] != '\0'); i++) wascmd = true;
    
    if ((plex->lex[i] != '\0') || wascmd) {
        ptok->tok = CMDNAME;
        if (plex->lex[i] != '\0') {
            plex->lex[i] = '\0';
            newlexem(plex, &(plex->lex[i+1]));
            newtoken(ptok, ARGLIST);
            ret++;
        }
    }
    return ret;
}

int parse_conv(lexem* plex, token* ptok) { 
    int ret = 0;
    int i;
    for (i = 0; (plex->lex[i] != '|') && (plex->lex[i] != '\0'); i++);
    if (plex->lex[i] == '|') {
        plex->lex[i] = '\0';
        newtoken(ptok, CONVSEP);
        newlexem(plex, &(plex->lex[i]));
        
        newtoken(ptok->ntok, CONV);
        newlexem(plex->nlex, &(plex->nlex->lex[1]));
        ret += parse_conv(plex->nlex->nlex, ptok->ntok->ntok);
    }
    return ret + parse_simplecmd(plex, ptok);
}

int parse_cmd(lexem* plex, token* ptok) {
    int ret = 0, i = 0;
    
    while (plex->lex[i] == ' ') i++;
    
    if ((plex->lex[i] == '<') || (plex->lex[i] == '>')) {
        parse_redirio(plex, ptok);
        while ((ptok->tok == REDIRI) || (ptok->tok == REDIRO) ||
                                            (ptok->tok == REDIROA)) {
            ptok = ptok->ntok;
            plex = plex->nlex;
        }
        parse_conv(plex, ptok);
    } else {
        int j = i;
        bool wascmd = false;
        while ((plex->lex[j] != '<') && (plex->lex[j] != '>')) {
            if (plex->lex[j] == '\0') break;
            if (isalpha(plex->lex[j])) wascmd = true;
            j++;
        }
        
        if (plex->lex[j] != '\0') {
            newlexem(plex, &(plex->lex[j]));
            newtoken(ptok, REDIRIO);
            parse_redirio(plex->nlex, ptok->ntok);
            plex->lex[j] = '\0';
        }
        if (wascmd) parse_conv(plex, ptok);   
        
         //plex->lex = &(plex->lex[i]);
         //
         
    }
    return ret;
}

int parse_condcmd(lexem* plex, token* ptok) {
    int ret = 0, i;
    ptok->tok = CMD;
    
    for ( i = 0; plex->lex[i] != '\0'; i++) {
        if ((plex->lex[i] == '&') && (plex->lex[i+1] == '&')) {
            newlexem(plex, &(plex->lex[i]));
            newtoken(ptok, CONDOK);
            break;
        }
        if ((plex->lex[i] == '|') && (plex->lex[i+1] == '|')) {
            newlexem(plex, &(plex->lex[i]));
            newtoken(ptok, CONDBAD);
            break;
        }
    }

    if (plex->lex[i] != '\0') {
        plex->lex[i] = '\0';
        newlexem(plex->nlex, &(plex->lex[i+2]));
        newtoken(ptok->ntok, CONDCMD);
        ret += parse_condcmd(plex->nlex->nlex, ptok->ntok->ntok);
    }
    ret += parse_cmd(plex, ptok);

    return ret;
}

int parse_dashcmd(lexem* plex, token* ptok) {
    int ret = 0, i = 0, wascmd = 0;

    while (plex->lex[i] != '\0') {
        if (plex->lex[i] == ';') break;
        
        if (plex->lex[i] == '&') {
            bool t = false;
            if (i > 0) {
                if (plex->lex[i-1] != '&') t = true;
            } else t = true;
            if ((plex->lex[i+1] != '&') && (t==true)) break;
        }
        /*
        if ((plex->lex[i-1] != '&') && (plex->lex[i] == '&') &&
                                        (plex->lex[i+1] != '&')) break;
        {*/
        if (plex->lex[i] == '(') break;
        if (isalpha(plex->lex[i])) wascmd = 1;
        i++;
    }
    
    
    if (plex->lex[i] == '\0') {
        ret += parse_condcmd(plex, ptok);
        return ret;
    }
    
    if (plex->lex[i] == '(') {
        int lastbr = -1, j;
        for (j = i; (plex->lex[j] != '\0'); j++)
            if (plex->lex[j] == ')') lastbr = j;
        if (lastbr == -1)
            panic(SYNTAX, "не хватает закрывающих скобок.");
        j = lastbr;
        // Ставим на её место нулевой байт
        plex->lex[i] = ' ';
        plex->lex[j] = '\0';
        
        newlexem(plex, &(plex->lex[j+1])); 
        newtoken(ptok, DASHCMD);
        parse_dashcmd(plex->nlex, ptok->ntok);
    }

    if (plex->lex[i] == ';') {
        plex->lex[i] = '\0';
        
        if (plex->lex[i+1] != '\0') {
            if (wascmd) {
                newlexem(plex, &(plex->lex[i+1]));
                newtoken(ptok, DASHCMD);
                    
                ret += parse_dashcmd(plex->nlex, ptok->ntok);
            } else plex->lex = &(plex->lex[i+1]);
        }
        ret += parse_condcmd(plex, ptok);
    }
    if (plex->lex[i] == '&') {
        bool t = false;
        if (i > 0) {
            if (plex->lex[i-1] != '&') t = true;
        } else t = true;
        if ((plex->lex[i+1] != '&') && (t==true)) {
            plex->lex[i] = '\0';
            ptok->tok = BACKGRND;
            newlexem(plex, plex->lex);
            newtoken(ptok, DASHCMD);
            plex->lex = malloc(sizeof(char));
            plex->lex[0] = '\0';
            
            if (plex->nlex->lex[i+1] != '\0') {
                newlexem(plex->nlex, &(plex->nlex->lex[i+1]));
                newtoken(ptok->ntok, DASHCMD);
                
                ret += parse_dashcmd(plex->nlex->nlex, ptok->ntok->ntok);
            }
        }
    }
    
    return ret;
}


/* На вход командная строка, на выход -- структура из токенов и
 * соответствующих им лексем.                                   */
cmdline tokenize(char* args) { 
    cmdline cmd = newcmdline(args);
    
    parse_dashcmd(cmd.lexems, cmd.tokens);
    
    return cmd;
}

#endif
