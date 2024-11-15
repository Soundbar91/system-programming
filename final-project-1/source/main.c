#include "config.h"
#include "commands.h"
#include "util.h"

char *current_dir = NULL;
char *chroot_path = "/tmp/test";

int main(int argc, char **argv)
{
    char *command, *tok_str;
    char *cmd_argv[MAX_ARG];
    char *current_dir;
    int cmd_argc, ret;

    init(command, current_dir);

    do
    {
        get_current_directory(current_dir, MAX_CMDLINE_SIZE);
        print_path(current_dir);

        if (fgets(command, MAX_CMD_SIZE - 1, stdin) == NULL)
        {
            break;
        }

        tok_str = strtok(command, " \n");
        if (tok_str == NULL)
        {
            continue;
        }
        cmd_argv[0] = tok_str;

        for (cmd_argc = 1; cmd_argc < MAX_ARG; cmd_argc++)
        {
            if ((tok_str = strtok(NULL, " \n")))
            {
                cmd_argv[cmd_argc] = tok_str;
            }
            else
            {
                break;
            }
        }

        int command_index = search_command(cmd_argv[0]);
        if (command_index < 0)
        {
            printf("%s: command not found\n", cmd_argv[0]);
        }
        else
        {
            if (cmd_list[command_index].cmd_func)
            {
                ret = cmd_list[command_index].cmd_func(cmd_argc, cmd_argv);
                if (ret)
                {
                    printf("return success\n");
                }
                else if (ret == -2 && cmd_list[command_index].usage_func)
                {
                    printf("usage : ");
                    cmd_list[command_index].usage_func();
                }
                else
                {
                    printf("return fail(%d)\n", ret);
                }
            }
            else
            {
                printf("no command function\n");
            }
        }
    } while (1);

    free(command);
    free(current_dir);

    return 0;
}
