#include "utils.h"
#include "commands.h"

int search_command(char *cmd)
{
    for (int i = 0; i < sizeof(cmd_list); i++) {
        if (strcmp(cmd, cmd_list[i].cmd_str) == 0)
        {
            return i;
        }
    }

    return -1;
}
