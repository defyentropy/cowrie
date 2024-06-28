#include <stdlib.h>
#include "../include/lexer.h"
#include "../include/parser.h"

SimpleCmd parse_simple_cmd(Parser *parser)
{
    size_t arg_buf_size = 16, arg_count = 0;
    SimpleCmd new_simple_cmd;
    new_simple_cmd.args = calloc(arg_buf_size, sizeof(char *));
    new_simple_cmd.infile = NULL;
    new_simple_cmd.outfile = NULL;
    int parse_error = 0;

    while (
        parser->current->type != TOKEN_PIPE &&
        parser->current->type != TOKEN_BG &&
        parser->current->type != TOKEN_SEMICOLON &&
        parser->current->type != TOKEN_END &&
        !parse_error
    )
    {
        switch (parser->current->type)
        {
            case TOKEN_IN:
                if ((parser->current + 1)->type != TOKEN_WORD)
                {
                    parse_error = 1;
                    break;
                }
                new_simple_cmd.infile = (parser->current + 1)->start;
                parser->current += 2;
                break;

            case TOKEN_OUT:
                if ((parser->current + 1)->type != TOKEN_WORD)
                {
                    parse_error = 1;
                    break;
                }
                new_simple_cmd.outfile = (parser->current + 1)->start;
                parser->current += 2;
                break;

            case TOKEN_WORD:
                new_simple_cmd.args[arg_count++] = parser->current->start;
                parser->current += 1;

                if (arg_count >= arg_buf_size)
                {
                    arg_count *= 2;
                    new_simple_cmd.args = reallocarray(new_simple_cmd.args, arg_count, sizeof(char *));
                }

            default:
                break;
        }
    }

    new_simple_cmd.args[arg_count++] = NULL;

    return new_simple_cmd;
}

PipeList parse_pipe_list(Parser *parser)
{
    size_t piped_cmds_buf_size = 16;
    PipeList piped_cmds;
    piped_cmds.cmds = malloc(piped_cmds_buf_size * sizeof(SimpleCmd));
    piped_cmds.bg = 0;
    piped_cmds.cmd_count = 0;

    while (
        parser->current->type != TOKEN_BG &&
        parser->current->type != TOKEN_SEMICOLON &&
        parser->current->type != TOKEN_END
    )
    {
        if (parser->current->type == TOKEN_PIPE)
        {
            parser->current += 1;
        }

        piped_cmds.cmds[piped_cmds.cmd_count++] = parse_simple_cmd(parser);

        if (piped_cmds.cmd_count >= piped_cmds_buf_size)
        {
            piped_cmds_buf_size *= 2;
            piped_cmds.cmds = realloc(piped_cmds.cmds, piped_cmds_buf_size * sizeof(SimpleCmd));
        }
    }

    if (parser->current->type == TOKEN_BG)
    {
        piped_cmds.bg = 1;
        parser->current += 1;
    }

    return piped_cmds;
}

CmdList parse_cmd_list(Parser *parser)
{
    size_t cmd_buf_size = 8;
    CmdList cmd_buf;
    cmd_buf.pipes = malloc(cmd_buf_size * sizeof(PipeList));
    cmd_buf.cmd_count = 0;

    while (parser->current->type != TOKEN_END)
    {
        if (parser->current->type == TOKEN_SEMICOLON)
        {
            parser->current += 1;
        }

        cmd_buf.pipes[cmd_buf.cmd_count++] = parse_pipe_list(parser);

        if (cmd_buf.cmd_count >= cmd_buf_size)
        {
            cmd_buf_size *= 2;
            cmd_buf.pipes = realloc(cmd_buf.pipes, cmd_buf_size * sizeof(PipeList));
        }
    }

    return cmd_buf;
}
