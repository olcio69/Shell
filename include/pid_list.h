//
// Created by Olgierd Zygmunt on 20/11/2024.
//
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef SHELL_PID_LIST_H
#define SHELL_PID_LIST_H

typedef struct pid_list{
    pid_t pids[MAX_PID];
    int current;
}pid_list;

void init(pid_list* b);
void remove_pid(pid_list *b, pid_t p);
int find_pid(pid_list *b, pid_t p);
void add_pid(pid_list* b, pid_t p);
#endif //SHELL_PID_LIST_H
