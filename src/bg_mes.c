#include "bg_mes.h"
#include <stdio.h>


void print_mes(bg_mes *b){
    for(int i = 0; i < b->current;i++){
        printf("Process %d ended in background\n",b->messeges[i]);
    }
    b->current = 0;
}
void add_mes(bg_mes* b, pid_t mes){
    if(b->current < MAX_MES) {
        b->messeges[b->current] = mes;
        b->current++;
    }//aby nie można było pisać poza tablicę.
}
void bg_init(bg_mes* b) {
    b->current = 0;
}
