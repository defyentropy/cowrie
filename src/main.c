#include <stdio.h>
#include <stdlib.h>
#include "../include/lexer.h"
#include "../include/parser.h"

int main(void)
{
    char *cmd_buf = NULL;
    size_t cmd_buf_size;
    ssize_t chars_read;
    Token *tokens;

    chars_read = getline(&cmd_buf, &cmd_buf_size, stdin);
    cmd_buf[--chars_read] = '\0';
    tokens = lex_line(cmd_buf);

    sanitise_cmds(tokens);
    free(cmd_buf);

    Parser parser;
    parser.start = tokens;
    parser.current = tokens;

    CmdList parsed_cmds = parse_cmd_list(&parser);

    for (size_t i = 0; i < parsed_cmds.cmd_count; i++)
    {
        puts("*-- piped list of commands --*");

        for (size_t j = 0; j < parsed_cmds.pipes[i].cmd_count; j++)
        {
            puts("*-- command --*");

            char **arg = parsed_cmds.pipes[i].cmds[j].args;

            while (*arg != NULL)
            {
                printf("%s ", *arg);
                arg++;
            }
            puts("");
            printf("infile: %s\n", parsed_cmds.pipes[i].cmds[j].infile);
            printf("outfile: %s\n", parsed_cmds.pipes[i].cmds[j].outfile);
        }

        printf("is bg: %d\n", parsed_cmds.pipes[i].bg);
    }


    for (size_t i = 0; i < parsed_cmds.cmd_count; i++)
    {
        for (size_t j = 0; j < parsed_cmds.pipes[i].cmd_count; j++)
        {
            free(parsed_cmds.pipes[i].cmds[j].args);
        }
        
        free(parsed_cmds.pipes[i].cmds);
    }

    free(parsed_cmds.pipes);

    free(tokens[0].start);
    free(tokens);
    return 0;
}
