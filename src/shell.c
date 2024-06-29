#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int exec_prog(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();

    switch (pid)
    {
        case 0:
            if (execvp(args[0], args) == -1)
            {
                perror("cowrie");
            }
            exit(EXIT_FAILURE);

        case -1:
            perror("cowrie");

        default:
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);    
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 0;
}
