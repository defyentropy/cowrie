#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "../include/builtins.h"
#include "../include/parser.h"

typedef int pipe_fds[2];

// manage file redirection and calling execvp
// reused in exec_prog and exec_pipeline
void handle_exec(char **args, char *infile, char *outfile)
{
    FILE *fd_infile = NULL, *fd_outfile = NULL;

    // reset SIGINT handlers so it can be interrupted
    signal(SIGINT, SIG_DFL);

    // open files and change stdin/stdout descriptors
    if (infile)
    {
        if ((fd_infile = fopen(infile, "r")) == NULL)
        {
            perror("cowrie: error opening file");
            exit(EXIT_FAILURE);
        }
    }

    if (outfile)
    {
        if ((fd_outfile = fopen(outfile, "w")) == NULL)
        {
            perror("cowrie: error opening file");
            exit(EXIT_FAILURE);
        }
    }

    if (fd_infile)
    {
        dup2(fileno(fd_infile), fileno(stdin));
    }

    if (fd_outfile)
    {
        dup2(fileno(fd_outfile), fileno(stdout));
    }

    if (execvp(args[0], args) == -1)
    {
        perror("cowrie");
    }
    exit(EXIT_FAILURE);
}

// exec a program from the PATH
int exec_prog(char **args, char *infile, char *outfile)
{
    pid_t pid, wpid;
    int status;

    pid = fork();

    switch (pid)
    {
        case 0:
            handle_exec(args, infile, outfile);

        case -1:
            perror("cowrie: error calling fork");
            return 1;

        default:
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);    

                if (wpid == -1)
                {
                    perror("cowrie: waitpid");
                    return 1;
                }

            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 0;
}

// either execute a builtin or a PATH program
int exec_cmd(SimpleCmd *cmd)
{
    if (is_builtin(cmd->args[0]) == 1)
    {
        return exec_builtin(cmd->args);
    }
    else
    {
        return exec_prog(cmd->args, cmd->infile, cmd->outfile);
    }
}

// execute a piped list of commands
int exec_pipeline(PipeList* pipeline)
{
    pid_t pid;
    int status;
    pipe_fds *pipes = malloc((pipeline->cmd_count - 1) * sizeof(pipe_fds));

    // create pipes between all the processes
    if (pipes == NULL)
    {
        perror("cowrie: allocation error");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < pipeline->cmd_count - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("cowrie: error opening pipe");
            exit(EXIT_FAILURE);
        }
    }

    // execute each command in its own child process
    for (size_t i = 0; i < pipeline->cmd_count; i++)
    {
        pid = fork();

        if (pid == 0)
        {
            // redirect input to pipes in child processes
            if (i > 0)
            {
                dup2(pipes[i - 1][0], fileno(stdin));
            }

            if (i < pipeline->cmd_count - 1)
            {
                dup2(pipes[i][1], fileno(stdout));
            }

            for (size_t j = 0; j < pipeline->cmd_count - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if (is_builtin(pipeline->cmds[i]->args[0]) == 1)
            {
                return exec_builtin(pipeline->cmds[i]->args);
            }
            else
            {
                handle_exec(
                    pipeline->cmds[i]->args, 
                    pipeline->cmds[i]->infile,
                    pipeline->cmds[i]->outfile
                );
            }
        }
        else if (pid < 0)
        {
            perror("cowrie: error calling fork");
            exit(EXIT_FAILURE);
        }
    }

    // close all pipes in the parent process
    for (size_t j = 0; j < pipeline->cmd_count - 1; j++)
    {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    // wait for the child processes to finish executing
    for (size_t i = 0; i < pipeline->cmd_count; i++)
    {
        wait(NULL);
    }

    free(pipes);

    return 0;
}
