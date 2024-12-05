#include "commands.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR_MESSAGE "kill error"

int cmd_quit(int argc, char **argv)
{
    if (argc == 2)
    {
        int signal = atoi(argv[1]);
        if (kill(getpid(), signal) < 0)
        {
            perror(ERROR_MESSAGE);
            return -1;
        }
    }
    else if (argc == 1)
    {
        if (kill(getpid(), SIGTERM) < 0)
        {
            perror(ERROR_MESSAGE);
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

void usage_quit(void)
{
    printf("quit [signal]\n");
    printf("Send the specified signal to the current process to terminate it.\n");
    printf("If no signal is specified, SIGTERM is used by default.\n");
    printf("Examples:\n");
    printf("  quit         # Terminate the process using SIGTERM\n");
    printf("  quit 9       # Terminate the process using SIGKILL\n");
}
