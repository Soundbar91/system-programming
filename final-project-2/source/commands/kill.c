#include "commands.h"
#include "utils.h"
#include <sys/types.h>
#include <signal.h>

int cmd_kill(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        return -2;
    }

    pid_t pid = atoi(argv[1]);
    if (pid <= 0)
    {
        perror("invalid PID");
        return -1;
    }

    int signal = SIGKILL;
    if (argc == 3)
    {
        signal = atoi(argv[2]);
        if (signal <= 0)
        {
            perror("invalid signal");
            return -1;
        }
    }
    if (kill(pid, SIGKILL) == 0)
    {
        printf("Process %d killed successfully.\n", pid);
        return 0;
    }
    else
    {
        perror("kill error");
        return -1;
    }
}

void usage_kill(void)
{
    printf("kill <PID> [SIGNAL]\n");
    printf("Sends a SIGKILL signal to the specified process ID (PID).\n");
}
