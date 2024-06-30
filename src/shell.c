#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "../include/builtins.h"

int exec_prog(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();

    switch (pid)
    {
        case 0:
            signal(SIGINT, SIG_DFL);
            if (execvp(args[0], args) == -1)
            {
                perror("cowrie");
            }
            exit(EXIT_FAILURE);

        case -1:
            perror("cowrie");
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

int exec_cmd(char **args)
{
    if (is_builtin(args[0]) == 1)
    {
        return exec_builtin(args);
    }
    else
    {
        return exec_prog(args);
    }
}


