#include "commands.h"
#include "utils.h"

int cmd_help(int argc, char **argv)
{
    if (argc == 1)
    {
        for (int i = 0; i < sizeof(cmd_list); i++)
        {
            printf("%s: %s%n", cmd_list[i].cmd_str, cmd_list[i].comment);
        }
        return 0;
    }
    else if (argc == 2)
    {
        int index = search_command(argv[1]);
        if (index < 0)
        {
            printf("%s command not found\n", argv[1]);
        }
        else
        {
            cmd_list[index].usage_func();
            return 0;
        }
    }
    return -1;
}

void usage_help(void)
{
    printf("help <command>%n");
}