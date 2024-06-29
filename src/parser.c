#include <stdio.h>
#include <stdlib.h>
#include "../include/lexer.h"
#include "../include/parser.h"

// parses one single command and returns a NULL-terminated array of pointers
// to the arguments, as well as information on infile and outfile redirection
SimpleCmd *parse_simple_cmd(Parser *parser)
{
    size_t arg_buf_size = 16, arg_count = 0;
    SimpleCmd *new_simple_cmd = malloc(sizeof(SimpleCmd));
    if (new_simple_cmd == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    new_simple_cmd->args = calloc(arg_buf_size, sizeof(char *));
    if (new_simple_cmd->args == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    new_simple_cmd->infile = NULL;
    new_simple_cmd->outfile = NULL;

    while (
        !parser->parse_error &&
        parser->current->type != TOKEN_PIPE &&
        parser->current->type != TOKEN_BG &&
        parser->current->type != TOKEN_SEMICOLON &&
        parser->current->type != TOKEN_END 
    )
    {
        switch (parser->current->type)
        {
            case TOKEN_IN:
                if ((parser->current + 1)->type != TOKEN_WORD)
                {
                    parser->parse_error = 1;
                    break;
                }
                new_simple_cmd->infile = (parser->current + 1)->start;
                parser->current += 2;
                break;

            case TOKEN_OUT:
                if ((parser->current + 1)->type != TOKEN_WORD)
                {
                    parser->parse_error = 1;
                    break;
                }
                new_simple_cmd->outfile = (parser->current + 1)->start;
                parser->current += 2;
                break;

            case TOKEN_WORD:
                new_simple_cmd->args[arg_count++] = parser->current->start;
                parser->current += 1;

                if (arg_count >= arg_buf_size)
                {
                    arg_count *= 2;
                    new_simple_cmd->args = reallocarray(new_simple_cmd->args, arg_count, sizeof(char *));

                    if (new_simple_cmd->args == NULL)
                    {
                        perror("cowrie: allocation error");
                        exit(EXIT_FAILURE);
                    }
                }

            default:
                break;
        }
    }

    if (parser->parse_error)
    {
        free(new_simple_cmd->args);
        free(new_simple_cmd);
        return NULL;
    }
    else
    {
        new_simple_cmd->args[arg_count++] = NULL;
        return new_simple_cmd;
    }
}

// returns an array of pointers to individual simple commands that need to
// connected together by pipes in order
PipeList *parse_pipe_list(Parser *parser)
{
    size_t piped_cmds_buf_size = 16;
    PipeList *piped_cmds = malloc(sizeof(PipeList));
    if (piped_cmds == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    piped_cmds->cmds = malloc(piped_cmds_buf_size * sizeof(SimpleCmd *));
    if (piped_cmds->cmds ==  NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    piped_cmds->bg = 0;
    piped_cmds->cmd_count = 0;
    SimpleCmd *next_cmd;

    while (
        !parser->parse_error &&
        parser->current->type != TOKEN_BG &&
        parser->current->type != TOKEN_SEMICOLON &&
        parser->current->type != TOKEN_END 
    )
    {
        if (parser->current->type == TOKEN_PIPE)
        {
            parser->current += 1;
        }
        else
        {

            next_cmd = parse_simple_cmd(parser);
            if (next_cmd == NULL)
            {
                break;
            }

            piped_cmds->cmds[piped_cmds->cmd_count++] = next_cmd; 

            if (piped_cmds->cmd_count >= piped_cmds_buf_size)
            {
                piped_cmds_buf_size *= 2;
                piped_cmds->cmds = realloc(
                    piped_cmds->cmds, 
                    piped_cmds_buf_size * sizeof(SimpleCmd *)
                );

                if (piped_cmds->cmds ==  NULL)
                {
                    perror("cowrie: allocation error");
                    exit(EXIT_FAILURE);
                }


            }
        }
    }

    if (parser->parse_error)
    {
        free(piped_cmds->cmds);
        free(piped_cmds);
        return NULL;
    }
    else
    {

        if (parser->current->type == TOKEN_BG)
        {
            piped_cmds->bg = 1;
            parser->current += 1;
        }

        return piped_cmds;
    }
}

// parses an entire line of command line input into multiple pipelines
CmdList *parse_cmd_list(Parser *parser)
{
    size_t cmd_buf_size = 8;
    CmdList *cmd_buf = malloc(sizeof(CmdList));
    if (cmd_buf == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    cmd_buf->pipes = malloc(cmd_buf_size * sizeof(PipeList *));
    if (cmd_buf->pipes == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    cmd_buf->cmd_count = 0;
    PipeList *next_pipe;

    while (parser->current->type != TOKEN_END)
    {
        if (parser->current->type == TOKEN_SEMICOLON)
        {
            parser->current += 1;
        }
        else
        {
            next_pipe = parse_pipe_list(parser);
            if (next_pipe == NULL)
            {
                break;
            }

            cmd_buf->pipes[cmd_buf->cmd_count++] = next_pipe;

            if (cmd_buf->cmd_count >= cmd_buf_size)
            {
                cmd_buf_size *= 2;
                cmd_buf->pipes = realloc(
                    cmd_buf->pipes, 
                    cmd_buf_size * sizeof(PipeList *)
                );

                if (cmd_buf->pipes == NULL)
                {
                    perror("cowrie: allocation error");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    if (parser->parse_error)
    {
        free(cmd_buf->pipes);
        free(cmd_buf);
        return NULL;
    }
    else
    {
        return cmd_buf;
    }
}
