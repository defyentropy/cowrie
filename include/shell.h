#ifndef SHELL_H
#define SHELL_H

#include "parser.h"

int exec_prog(char **args);
int exec_cmd(SimpleCmd *cmd);
int exec_pipeline(PipeList* pipeline);

#endif
