#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#include "errno.h"
#include "config.h"
#include "siparse.h"
#include "utils.h"

int buf_errors(int w){
    if(w > MAX_LINE_LENGTH) {
        fprintf(stderr,"Syntax error.\n");
        return 1;
    }
    if(w == 0){
        return 2;
    }
    return 0;
}

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

void trim_leading_spaces(char *str) {
    char *ptr = str;
    while (isspace(*ptr)) {
        ptr++;
    }

    if (ptr != str) {
        char *temp = str;
        while (*ptr) {
            *temp++ = *ptr++;
        }
        *temp = '\0';
    }
}

void check_and_execute(char* exec_buf){
    trim_leading_spaces(exec_buf);
    pipelineseq *ln = parseline(exec_buf);
    command *com = pickfirstcommand(ln);
    if(exec_buf[0] != 0 && exec_buf[0] != '#') {
        //if(com == 0) printf("works");
        argseq *first_arg = com->args;
        pid_t pid = fork();
        if (pid == 0) {
            execute_command(first_arg, com);
        }
        wait(NULL);
    }
}

int
main(int argc, char *argv[])
{
    int z = 0, overflow_mode = 0;
    struct stat buffer;
    fstat(0, &buffer);
    int file_input_mode = 0;
    if(buffer.st_rdev == 0) file_input_mode = 1;

    int sep = 0;
    char buf[MAX_LINE_LENGTH];
    char exec_buf[MAX_LINE_LENGTH];
    exec_buf[0] = 0;

    while (1) {
        sep = 0;
        if (!file_input_mode) {
            printf("%s", PROMPT_STR);
            fflush(stdout);
            sep = 0;
        }
        int w = read(0, buf, MAX_LINE_LENGTH);
        int err = buf_errors(w);
        if (err == 1) break;
        if (err == 2) {
            check_and_execute(exec_buf);
            break;
        }
        for (; sep < w; sep++) {
            exec_buf[z] = buf[sep];
            ++z;
            if (buf[sep] == '\n') {
                exec_buf[z - 1] = 0;
                z = 0;
                if(!overflow_mode) check_and_execute(exec_buf);
                exec_buf[0] = 0;
                if(overflow_mode) overflow_mode = 0;
            }
            if(z >= 2048){
                exec_buf[0] = 0;
                z = 0;
                if(!overflow_mode) {
                    fprintf(stderr, "Syntax error.\n");
                    overflow_mode = 1;
                }
            }
        }
    }
    return 0;
}