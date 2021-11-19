#include <stdio.h>
#include "token.h"
#include "run.h"
#include "sigh.h"

int main(int argc, char** argv) {
    sigign();
    initjobs();
    
    FILE* s = NULL;
    if (argc == 2) {
        s = fopen(argv[1], "r");
        if (s == NULL) {
            // 26 == strlen("не удалось открыть файл .") + 1
            char* msg = malloc((26 + strlen(argv[1])) * sizeof(char));
            sprintf(msg, "не удалось открыть файл %s.", argv[1]);
            panic(FLAGS, msg);
        }
        do { job(s); } forever;
    } else if (argc == 1) {
        printf("Добро пожаловать в dash!\n");
        do { job(stdin); } forever;
    } else panic(FLAGS, "неверное количество.");
    
    return 0;
}
