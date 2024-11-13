//
// Created by Olgierd Zygmunt on 04/11/2024.
//

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "error_detection.h"
#include "exec_utils.h"
#include "siparse.h"

int get_arg_redir_len(command *com, int mode){
    if(mode == 0){ //mode 0 -> arg, mode 1 -> redir
        int i = 1;
        argseq *first_arg = com -> args;
        while (1) {
            com->args = com->args->next;
            if (com->args == first_arg) break;
            ++i;
        }
        return i;
    }
    if(mode == 1){
        int i = 0;
        redirseq *first_redir = com->redirs;
        if(first_redir != NULL){
            while (1) {
                ++i;
                com->redirs = com->redirs->next;
                if (com->redirs == first_redir) break;
            }
        }
        return i;
    }
    return -1;
}
int get_pipeseq_len(pipelineseq *p){
    int i = 0;
    pipelineseq *first = p;
    if(first != NULL){
        while(1){
            ++i;
            p = p->next;
            if(p == first) break;
        }
    }
    return i;
}
int get_pipe_len(pipeline *p){
    int i = 0;
    commandseq *first = p->commands;
    if(first != NULL){
        while(1){
            ++i;
            p->commands = p->commands->next;
            if(p->commands == first) break;
        }
    }
    return i;
}
void fill_arg(char *arguments[],int args_len, command *com){
    for (int j = 0; j < args_len; j++) {
        arguments[j] = com->args->arg;
        com->args = com->args->next;
    }
    arguments[args_len] = NULL;
}
void fill_redir(redir* redirs[], int redirs_len, command *com){
    for(int j = 0; j < redirs_len; j++){
        redirs[j] = com->redirs->r;
        com->redirs = com->redirs->next;
    }
    redirs[redirs_len] = NULL;
}
void fill_pipe(pipeline* pips[], int pip_len, pipelineseq *pipseq){
    for(int j = 0; j < pip_len; j++){
        pips[j] = pipseq->pipeline;
        pipseq = pipseq->next;
    }
    pips[pip_len] = NULL;
}

int open_w(redir* red) {
    errno = 0;
    int w = 0;
    if (IS_RAPPEND(red->flags) || IS_ROUT(red->flags)) {
        if (IS_RAPPEND(red->flags)) {
            w = open(red->filename, O_WRONLY | O_APPEND | O_CREAT, 0666); //dziala
        } else if (IS_ROUT(red->flags)) {
            w = open(red->filename, O_WRONLY | O_TRUNC | O_CREAT, 0666); //dziala
        }
        if (w == -1) {
            file_err(red);
            close(w);
            return -1;
        }
        else dup2(w, 1);
        close(w);
        return 1;
    }
    return 0;
}

int open_r(redir* red){
    errno = 0;
    int w = 0;
    if(IS_RIN(red->flags)){
        w = open(red->filename,O_RDONLY); //dziala
        if (w == -1) {
            file_err(red);
            close(w);
            return -1;
        }
        else dup2(w,0);
        close(w);
        return 1;
    }
    return 0;
}