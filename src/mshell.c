#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "errno.h"
#include "config.h"
#include "siparse.h"
#include "utils.h"

void execute_command(argseq *first_arg ,command *com){
    int i = 1;
    while(1){
        com->args = com->args->next;
        if(com->args == first_arg) break;
        ++i;
    }
    char* arguments[i+1];
    for(int j = 0; j < i;j++){
        arguments[j] = com->args->arg;
        com->args = com->args->next;
    }
    arguments[i] = NULL;
    int k = execvp(arguments[0], arguments);
    if(k == -1){
        if(errno == EACCES) fprintf(stderr,"%s: permission denied\n", arguments[0]);
        else if(errno == ENOENT) fprintf(stderr,"%s: no such file or directory\n", arguments[0]);
        else fprintf(stderr,"%s exec error\n", arguments[0]);
        exit(EXEC_FAILURE);
    }
}


int
main(int argc, char *argv[]) {
    pipelineseq *ln;
    command *com;

    char buf[MAX_LINE_LENGTH];

    while (1) {
        printf("%s", PROMPT_STR);
        fflush(stdout);
        int w = read(0, buf, MAX_LINE_LENGTH);
        if (w >= MAX_LINE_LENGTH) {
            printf("syntax error \n");
            continue;
        }
        if (w == 0) {
            printf("EOF reached\n");
            break;
        }
        buf[w - 1] = '\0';
        ln = parseline(buf);
        com = pickfirstcommand(ln);
        argseq *first_arg = com->args;
        pid_t pid = fork();
        if (pid == 0) {
            execute_command(first_arg, com);
        }
        wait(NULL);
    }

    return 0;
}