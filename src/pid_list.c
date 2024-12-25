#include "pid_list.h"

void init(pid_list* b){
    b->current = 0;
}

void add_pid(pid_list* b, pid_t p){
    if(b->current < MAX_PID) {
        b->pids[b->current] = p;
        b->current++;
    }
};

int find_pid(pid_list *b, pid_t p){
    for(int i = 0; i < b->current; i++){
        if(b->pids[i] == p) return i;
    }
    return -1;
}

void remove_pid(pid_list *b, pid_t p){
    int ix = find_pid(b,p);
    if(ix == -1) return;
    b->pids[ix] = b->pids[b->current-1];
    b->current--;
}

