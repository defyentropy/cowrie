#ifndef PARSER_H
#define PARSER_H

#include "./lexer.h"
#include <stdlib.h>

typedef struct simple_cmd
{
    char **args;
    char *infile;
    char *outfile;
} SimpleCmd;

typedef struct pipe_list
{
    SimpleCmd **cmds;
    int bg;
    size_t cmd_count;
} PipeList;

typedef struct cmd_list
{
    PipeList **pipes;
    size_t cmd_count;
} CmdList;

typedef struct parser
{
    Token *start;
    Token *current;
    int parse_error;
} Parser;

SimpleCmd *parse_simple_cmd(Parser *parser);
PipeList *parse_pipe_list(Parser *parser);
CmdList *parse_cmd_list(Parser *parser);

#endif
