#include "shell.h"
#include "commands.h"
#include "utils.h"
#include <signal.h>

/**
 * 2020136073 신관규
 * main.c
 */

void setup_signal_handling()
{
    if (signal(SIGINT, SIG_IGN) == SIG_ERR)
    {
        perror("signal error");
        exit(1);
    }
}

char *chroot_path = "/tmp/test";
char *current_dir;

int main(int argc, char **argv)
{
    char command[MAX_CMDLINE_SIZE];
    char *cmd_argv[MAX_ARG];
    int cmd_argc;

    setup_signal_handling();

    current_dir = malloc(MAX_CMDLINE_SIZE);
    if (!current_dir)
    {
        perror("malloc");
        return 1;
    }

    if (chdir(chroot_path) < 0)
    {
        mkdir(chroot_path, 0755);
        chdir(chroot_path);
    }

    while (1)
    {
        if (getcwd(current_dir, MAX_CMDLINE_SIZE) == NULL)
        {
            perror("getcwd");
            break;
        }

        printf("%s$ ", current_dir + strlen(chroot_path));
        if (!fgets(command, MAX_CMDLINE_SIZE, stdin))
            break;

        char *tok = strtok(command, " \n");
        if (!tok) {
            continue;
        }

        cmd_argv[0] = tok;
        for (cmd_argc = 1; cmd_argc < MAX_ARG; cmd_argc++)
        {
            if (!(tok = strtok(NULL, " \n"))) {
                break;
            }
            cmd_argv[cmd_argc] = tok;
        }

        int cmd_index = search_command(cmd_argv[0]);
        if (cmd_index < 0)
        {
            printf("%s: command not found\n", cmd_argv[0]);
        }
        else
        {
            int result = cmd_list[cmd_index].cmd_func(cmd_argc, cmd_argv);
            if (result == -2) {
                perror("argument valid");
            }
        }
    }

    free(current_dir);
    return 0;
}
