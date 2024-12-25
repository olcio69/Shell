#ifndef SHELL_BG_MES_H
#define SHELL_BG_MES_H

#include "config.h"
#include <stdlib.h>

typedef struct bg_mes{
    int messeges[MAX_MES];
    int current;
}bg_mes;

void bg_init(bg_mes* b);
void add_mes(bg_mes* b, pid_t mes);
void print_mes(bg_mes *b);

#endif //SHELL_BG_MES_H
