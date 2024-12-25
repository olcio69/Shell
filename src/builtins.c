#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>

#include "builtins.h"

int exi_(char*[]);
int lkill(char*[]);
int lls(char *[]);
int lcd(char *[]);

builtin_pair builtins_table[]={
	{"exit",	&exi_},
	{"lcd",		&lcd},
	{"lkill",	&lkill},
	{"lls",		&lls},
	{NULL,NULL}
};

int bi_get_size(builtin_pair* b){
    int i = 0;
    while( (b+i)->name != NULL) ++i;
    return i;
}

static void err(char *s){
    fprintf(stderr, "Builtin %s error.\n", s);
}
static int get_argc(char * argv[]){
    int argc = 1;
    while(argv[argc] != NULL) ++argc;
    return argc;
}

int lls(char * argv[]){
    int argc = get_argc(argv);
    if(argc > 1){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    DIR *dirp;
    struct dirent *dp;

    if ((dirp = opendir(".")) == NULL) {
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (dp->d_name[0] == '.')
                continue;
            printf("%s\n", dp->d_name);
            fflush(stdout);
            if(errno != 0) {
                err(argv[0]);
                return BUILTIN_ERROR;
            }
        }
    } while (dp != NULL);
    closedir(dirp);
    return 0;
}

int exi_(char * argv[]){
    exit(EXIT_SUCCESS);
}

int lkill(char * argv[]){
    int argc = get_argc(argv);
    if(argc > 3 || argc < 2){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    errno = 0;
    long signal,process;
    char * endptr;
    char * endptr2;
    if(argc == 3) {
        if (argv[1][0] != '-') {
            err(argv[0]);
            return BUILTIN_ERROR;
        }
        process = strtol(argv[2], &endptr, 10);
        signal = strtol(argv[1] + 1, &endptr2, 10);
    }
    else if(argc == 2){
        signal = SIGINT;
        process = strtol(argv[1],&endptr,10);
    }
    if((signal == 0 && *endptr2 != '\0') || (process == 0 && *endptr != '\0') || ((signal == LONG_MAX || process == LONG_MAX) && errno != 0)){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    if(process > INT_MAX || process < INT_MIN || signal > INT_MAX || signal < INT_MIN){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    kill((int)process,(int)signal);
    return 0;
}


int lcd(char * argv[]){
    char *home_dir = getenv("HOME");
    int argc = get_argc(argv);
    if(argc > 2){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    int z;
    if(argc == 1) z = chdir(home_dir);
    else if(strcmp(argv[1],"~") == 0) z = chdir(home_dir);
    else z = chdir(argv[1]);
    if(z == -1){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    return 0;
}

