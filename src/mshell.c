#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "builtins.h"
#include "error_detection.h"
#include "exec_utils.h"

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

int exec_builtins(char * arguments[]){
    int last = bi_get_size(builtins_table);
    for (int j = 0; j < last; j++) {
        if (strcmp(arguments[0], builtins_table[j].name) == 0) {
            int w = builtins_table[j].fun(arguments);
            return 1;
        }
    }
    return 0;
}

void exec(char *arguments[], redir* rs[]){
    int i = 0,flag = 0;
    while(rs[i] != NULL){
        if(open_r(rs[i]) == -1 || open_w(rs[i]) == -1) flag = 1;
        if(flag){
            break;
        }
        ++i;
    }
    if(!flag) {
        int k = execvp(arguments[0], arguments);
        exec_err(k, arguments[0]);
    }
}

void check_and_execute(char* exec_buf) {
    trim_leading_spaces(exec_buf);
    pipelineseq *ln = parseline(exec_buf);
    int p_len = get_pipeseq_len(ln);
    pipeline *pips[p_len+1];
    fill_pipe(pips,p_len, ln);
    //printf("plen = %d\n", p_len);
    for(int i = 0; i < p_len; i++) {
        //fprintf(stderr,"%d\n", i);
        command *first_com = pickfirstcommand(ln);
        command *com = pickfirstcommand(ln);
        //printf("pipe len: ??? \n");
        int pipe_len = get_pipe_len(pips[i]);
        //printf("pipe len: %d\n", pipe_len);
        int prev_fd = -1;
        for (int j = 0; j < pipe_len; j++) {
            int fildes[2];
            pipe(fildes);
            int status;
            if (exec_buf[0] != 0 && exec_buf[0] != '#') {
                pid_t pid = fork();
                if (pid == 0) {
                    //printcommand(com,j);
                    if (prev_fd != -1) {
                        dup2(prev_fd, STDIN_FILENO);
                        close(prev_fd);
                    }
                    if (j != pipe_len - 1) {
                        dup2(fildes[1], STDOUT_FILENO);
                        close(fildes[1]);
                        close(fildes[0]);
                    }
                    int i = get_arg_redir_len(com, 0);
                    int z = get_arg_redir_len(com, 1);
                    redir *redirs[z + 1];
                    char *arguments[i + 1];
                    fill_arg(arguments, i, com);
                    fill_redir(redirs, z, com);
                    if (exec_builtins(arguments)) return;
                    exec(arguments, redirs);
                } else {
                    if (prev_fd != -1) close(prev_fd);
                    if (j < pipe_len - 1) {
                        close(fildes[1]);
                        prev_fd = fildes[0];
                    } else {
                        close(fildes[0]);
                        close(fildes[1]);
                    }
                }
                ln->pipeline->commands = ln->pipeline->commands->next;
                com = pickfirstcommand(ln);
                if (com == first_com) break;
            }
        }
        for(int i = 0; i < pipe_len;i++) wait(NULL);
        ln = ln->next;
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
        int err = buf_err(w);
        if (err == 1) break;
        if (err == 2) {
            check_and_execute(exec_buf);
            break;
        }
        for (; sep < w; sep++) {
            exec_buf[z] = buf[sep]; //docelowo memcpy
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
