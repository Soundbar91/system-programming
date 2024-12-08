#include "commands.h"
#include "utils.h"

int cmd_run(int argc, char **argv)
{
    if (argc < 2)
    {
        return -2;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        if (execvp(argv[1], &argv[1]) < 0)
        {
            perror("execvp error");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("fork error");
        return -1;
    }

    return 0;
}

void usage_run(void)
{
    printf("run <program> [args...]\n");
    printf("Runs the specified program with optional arguments.\n");
}
