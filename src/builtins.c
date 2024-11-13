#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>

#include "builtins.h"

void err(char* s);
int echo(char*[]);
int exi_(char*[]);
int lkill(char*[]);
int lls(char *[]);
int lcd(char *[]);

builtin_pair builtins_table[]={
	{"exit",	&exi_},
	{"lecho",	&echo},
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

void err(char *s){
    fprintf(stderr, "Builtin %s error.\n", s);
}
int get_argc(char * argv[]){
    int argc = 1;
    while(argv[argc] != NULL) ++argc;
    return argc;
}

int 
echo(char * argv[])
{
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
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
            if(errno != 0) {
                err(argv[0]);
                return BUILTIN_ERROR;
            }
        }
    } while (dp != NULL);
    printf("\n");
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
    int signal,process;
    if(argc == 3) {
        if (argv[1][0] != '-') {
            err(argv[0]);
            return BUILTIN_ERROR;
        }
        process = strtol(argv[2], NULL, 10);
        signal = strtol(argv[1] + 1, NULL, 10);
    }
    else if(argc == 2){
        signal = SIGINT;
        process = strtol(argv[1],NULL,10);
    }
    if((signal == 0 || process == 0) && errno != 0){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    kill(process,signal);
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
    else if(strcmp(argv[1],"~") == 0 || strcmp(argv[1],"home") == 0) z = chdir(home_dir);
    else z = chdir(argv[1]);
    if(z == -1){
        err(argv[0]);
        return BUILTIN_ERROR;
    }
    return 0;
}


int 
undefined(char * argv[])
{
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}
