//
// Created by Olgierd Zygmunt on 04/11/2024.
//
#include "siparse.h"

#ifndef SHELL_ERROR_DETECTION_H
#define SHELL_ERROR_DETECTION_H

void file_err(redir *red);

void exec_err(int k, char* name);

int buf_err(int w);

#endif //SHELL_ERROR_DETECTION_H
