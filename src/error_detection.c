//
// Created by Olgierd Zygmunt on 04/11/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "error_detection.h"
#include "siparse.h"

void exec_err(int k, char* name) {
    if(k != -1) return;
    if (errno == EACCES) fprintf(stderr, "%s: permission denied\n", name);
    else if (errno == ENOENT) fprintf(stderr, "%s: no such file or directory\n", name);
    else fprintf(stderr, "%s exec error\n", name);
    exit(EXEC_FAILURE);
}

void file_err(redir *red){
    if(errno == EACCES) fprintf(stderr, "%s: permission denied\n",red->filename);
    if(errno == ENOENT) fprintf(stderr, "%s: no such file or directory\n",red->filename);
    _exit(EXEC_FAILURE);
}

int buf_err(int w){
    if(w > MAX_LINE_LENGTH) {
        fprintf(stderr,"Syntax error.\n");
        return 1;
    }
    if(w == 0){
        return 2;
    }
    return 0;
}
