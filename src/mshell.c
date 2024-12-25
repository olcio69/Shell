#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>
#include <sys/wait.h>

#include "pid_list.h"
#include "bg_mes.h"
#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "builtins.h"
#include "error_detection.h"
#include "exec_utils.h"

unsigned int fg_count = 0;

bg_mes bgMes;
pid_list pidList;

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

void check_and_execute(char* exec_buf, sigset_t set) {
    trim_leading_spaces(exec_buf);
    pipelineseq *ln = parseline(exec_buf);
    int p_len = get_pipeseq_len(ln);
    pipeline *pips[p_len+1];
    fill_pipe(pips,p_len, ln);
    if(check_for_nulls(pips, p_len) == -1){
        buf_err(INT_MAX);
        return;
    }
    int fildes[2];
    int prev_fd = -1;

    for(int i = 0; i < p_len; i++) {
        int in_bg = 0;
        if(ln->pipeline->flags == INBACKGROUND) in_bg = 1;

        command *first_com = pickfirstcommand(ln);
        command *com = pickfirstcommand(ln);
        int pipe_len = get_pipe_len(pips[i]);
        for (int j = 0; j < pipe_len; j++) {
            if (exec_buf[0] != 0 && exec_buf[0] != '#') {

                int i = get_arg_redir_len(com, 0);
                int z = get_arg_redir_len(com, 1);
                redir *redirs[z + 1];
                char *arguments[i + 1];
                fill_arg(arguments, i, com);
                fill_redir(redirs, z, com);

                if (exec_builtins(arguments)) continue;

                if(j != pipe_len -1) pipe(fildes);
                sigprocmask(SIG_BLOCK,&set,NULL);
                pid_t pid = fork();
                if (pid == 0) {
                    struct sigaction sa_def;
                    sa_def.sa_handler = SIG_DFL;
                    sigemptyset(&sa_def.sa_mask);
                    sa_def.sa_flags = 0;
                    sigaction(SIGINT, &sa_def, NULL); //idea jest taka zeby nie CTRl-C nie wyłączał shella,
                    //wiec jak wchodzimy do procesu potomnego, to włączany defaultowy handling SIGINTA.
                    if(pipe_len > 1) {
                        if (prev_fd != -1) {
                            dup2(prev_fd, STDIN_FILENO);
                            close(prev_fd);
                        }
                        if (j != pipe_len - 1) {
                            dup2(fildes[1], STDOUT_FILENO);
                            close(fildes[1]);
                        }
                        close(fildes[0]);
                    }
                    if(in_bg) setsid();
                    exec(arguments, redirs);
                } else if(pipe_len > 1){
                    if (prev_fd != -1) close(prev_fd);
                    if(j != pipe_len -1) {
                        close(fildes[1]);
                        prev_fd = fildes[0];
                    } else if(pipe_len > 1 && j == pipe_len - 1){
                        close(fildes[0]);
                        close(fildes[1]);
                    }
                }
                if(!in_bg){
                    ++fg_count;
                } else {
                    add_pid(&pidList,pid); //chcemy trzymac procesy w backgroundzie
                }
                sigprocmask(SIG_UNBLOCK,&set,NULL);
                ln->pipeline->commands = ln->pipeline->commands->next;
                com = ln->pipeline->commands->com;
                if (com == first_com) break;
            }
        }
        sigset_t sus_set;
        sigemptyset(&sus_set);
        sigprocmask(SIG_BLOCK,&set,NULL);
        while(fg_count > 0){
            sigsuspend(&sus_set);
        }
        sigprocmask(SIG_UNBLOCK,&set,NULL);
        ln = ln->next;
        prev_fd = -1;
    }
    return;
}

void handler(int h){
    int status;
    int ch;
    do{
        ch = waitpid(-1,&status,WNOHANG);
        if(ch <= 0) break;
        if(ch > 0) {
            if(find_pid(&pidList,ch) >= 0) {
                remove_pid(&pidList,ch);
                add_mes(&bgMes, ch); //proces z backgroundu
            } else {
                --fg_count;
            }
        }
    }while(ch != -1);
}



int
main(int argc, char *argv[])
{
    bg_init(&bgMes);
    init(&pidList);

    struct sigaction sa_ignore;
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = 0;
    sigaction(SIGINT, &sa_ignore, NULL);
    //tutaj sobie to blokujemy a potem przy każdym forku będziemy odblokowywać

    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sigset_t set = sa.sa_mask;
    sigaction(SIGCHLD, &sa, NULL);

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
        int w;
        do {
            w = read(0, buf, MAX_LINE_LENGTH);
        }while(w < 0);
        if(!file_input_mode) {
            print_mes(&bgMes);
        }
        int err = buf_err(w);
        if (err == 1) break;
        if (err == 2) {
            check_and_execute(exec_buf,set);
            break;
        }
        for (; sep < w; sep++) {
            int rem = w - sep;
            int copy_len = rem;

            char *newline = memchr(&buf[sep], '\n', rem);
            if (newline) {
                copy_len = newline - &buf[sep] + 1;
            }
            if (copy_len + z > MAX_LINE_LENGTH) {
                copy_len = MAX_LINE_LENGTH - z;
            }
            memcpy(&exec_buf[z], &buf[sep], copy_len);
            z += copy_len;
            sep += copy_len - 1;

            if (newline && z > 0 && exec_buf[z - 1] == '\n') {
                exec_buf[z - 1] = 0;
                z = 0;
                if (!overflow_mode) check_and_execute(exec_buf, set);
                exec_buf[0] = 0;
                if (overflow_mode) overflow_mode = 0;
            }
            if (z >= MAX_LINE_LENGTH) {
                exec_buf[0] = 0;
                z = 0;
                if (!overflow_mode) {
                    fprintf(stderr, "Syntax error.\n");
                    overflow_mode = 1;
                }
            }
        }
    }
    return 0;

}
