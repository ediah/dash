#ifndef PANIC_H
#define PANIC_H

#define UNUSED(x) x=x;
// TODO: Panic()

/* Тип ошибки */
enum error { 
    FLAGS,   // Аргументы запуска шелла
    SYNTAX,  // Синтаксическая
    RUN,     // Выполнения
    UNKNOWN  // Неизвестная
};

void panic(enum error err, const char * msg) { 
    switch (err) {
        case FLAGS:
            printf("Неверные аргументы: ");
            break;
        case SYNTAX:
            printf("Синтаксическая ошибка: ");
            break;
        case RUN:
            printf("Ошибка выполнения: ");
            break;
        case UNKNOWN:
            printf("Неизвестная ошибка: ");
            break;
    }
    printf("%s\n", msg);
    free(msg);
    exit(1);
}
/***************/

#endif
