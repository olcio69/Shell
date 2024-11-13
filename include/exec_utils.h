//
// Created by Olgierd Zygmunt on 04/11/2024.
//

#ifndef SHELL_EXEC_UTILS_H
#define SHELL_EXEC_UTILS_H

#include "siparse.h"

void fill_arg(char *arguments[],int args_len, command *com);

void fill_redir(redir* redirs[], int redirs_len, command *com);

void fill_pipe(pipeline* pips[], int pip_len, pipelineseq *pipseq);

int get_arg_redir_len(command *com, int mode);

int get_pipeseq_len(pipelineseq *p);

int get_pipe_len(pipeline *p);

int open_w(redir* red);

int open_r(redir* red);


#endif //SHELL_EXEC_UTILS_H
