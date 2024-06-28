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
    SimpleCmd test_cmd = parse_simple_cmd(&parser);

    char **arg = test_cmd.args;
    while (*arg != NULL)
    {
        printf("%s\n", *arg);
        arg++;
    }

    free(tokens[0].start);
    free(tokens);
    return 0;
}
