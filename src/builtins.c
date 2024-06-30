#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUILTIN_COUNT 2

static int shush_cd(char **args);
static int shush_exit(char **args);

typedef struct shush_builtin
{
    char *name;
    int (*func)(char **);
} Shush_builtin;

Shush_builtin builtins[BUILTIN_COUNT] = {
    {"cd", shush_cd},
    {"exit", shush_exit}
};

int is_builtin(char cmd[])
{
    for (int i = 0; i < BUILTIN_COUNT; i++)
    {
        if (strcmp(builtins[i].name, cmd) == 0)
        {
            return 1;
        }
    }

    return 0;
}

int exec_builtin(char **args)
{
    for (int i = 0; i < BUILTIN_COUNT; i++)
    {
        if (strcmp(builtins[i].name, args[0]) == 0)
        {
            return builtins[i].func(args);
        }
    }

    return 0;
}

static int shush_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "cd: expected argument to 'cd'\n");
        return 1;
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("shush");
            return 1;
        }
    }

    return 0;
}

static int shush_exit(char **args)
{
    if (args[1] != NULL)
    {
        fprintf(stderr, "exit: too many arguments\n");
        return 1;
    }

    exit(EXIT_SUCCESS);
}
