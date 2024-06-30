#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/shell.h"

static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;

void print_prompt();
void sigint_handler(int sig_no)
{
    if (!jump_active)
    {
        return;
    }
    siglongjmp(env, 19);
}

int main(void)
{
    char *cmd_buf = NULL;
    size_t cmd_buf_size;
    ssize_t chars_read;
    Token *tokens;

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    while (1)
    {
        if (sigsetjmp(env, 1) == 19)
        {
            puts("");
        }
        jump_active = 1;

        print_prompt();

        chars_read = getline(&cmd_buf, &cmd_buf_size, stdin);
        // handle errors in getline
        if (chars_read == -1)
        {
            if (feof(stdin))
            {
                puts("\nexit");
                exit(EXIT_SUCCESS);
            }
            else
            {
                perror("cowrie: error reading input");
                exit(EXIT_FAILURE);
            }
        }
        // handle empty inputs
        else if (chars_read == 1)
        {

            free(cmd_buf);
            cmd_buf = NULL;
            continue;
        }

        cmd_buf[--chars_read] = '\0';

        // split into tokens, write tokens into new array for execvp call,
        // and free original buffer
        tokens = lex_line(cmd_buf);
        sanitise_cmds(tokens);
        free(cmd_buf);
        cmd_buf = NULL;

        Parser parser;
        parser.start = tokens;
        parser.current = tokens;
        parser.parse_error = 0;

        CmdList *parsed_cmds = parse_cmd_list(&parser);
        if (parsed_cmds == NULL)
        {
            printf("cowrie: parse error near '%s'\n", parser.current->start);
            free(tokens[0].start);
            free(tokens);
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < parsed_cmds->cmd_count; i++)
        {
            int status = exec_cmd(parsed_cmds->pipes[i]->cmds[0]->args);
        }
        puts("");

        /* for (size_t i = 0; i < parsed_cmds->cmd_count; i++)
        {
            puts("*-- piped list of commands --*");

            for (size_t j = 0; j < parsed_cmds->pipes[i]->cmd_count; j++)
            {
                puts("*-- command --*");

                char **arg = parsed_cmds->pipes[i]->cmds[j]->args;

                while (*arg != NULL)
                {
                    printf("%s ", *arg);
                    arg++;
                }
                puts("");
                printf("infile: %s\n", parsed_cmds->pipes[i]->cmds[j]->infile);
                printf("outfile: %s\n", parsed_cmds->pipes[i]->cmds[j]->outfile);
            }

            printf("is bg: %d\n", parsed_cmds->pipes[i]->bg);
        } */


        for (size_t i = 0; i < parsed_cmds->cmd_count; i++)
        {
            for (size_t j = 0; j < parsed_cmds->pipes[i]->cmd_count; j++)
            {
                free(parsed_cmds->pipes[i]->cmds[j]->args);
            }

            free(parsed_cmds->pipes[i]->cmds);
        }

        free(parsed_cmds->pipes);

        free(tokens[0].start);
        free(tokens);
    }
    return 0;
}


void print_prompt()
{

    size_t prompt_size = 128;
    char *prompt = malloc(prompt_size * sizeof(char));
    if (prompt == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    do
    {
        if (getcwd(prompt, prompt_size) != NULL)
        {
            break;
        }
        else
        {
            prompt_size *= 2;
            prompt = malloc(prompt_size * (sizeof(char)));
            if (prompt == NULL)
            {
                perror("cowrie: allocation error");
                exit(EXIT_FAILURE);
            }
        }
    } while (prompt_size < 1024);

    if (prompt == NULL)
    {
        printf("cowrie$ ");
    }
    else
    {
        printf("%s$ ", prompt);
    }
}
