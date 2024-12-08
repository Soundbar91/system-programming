#include "commands.h"
#include "utils.h"

int cmd_rmdir(int argc, char **argv)
{
    char real_path[MAX_PATH_SIZE];
    if (argc == 2)
    {
        get_realpath(argv[1], real_path);
        if (rmdir(real_path) < 0) {
            perror(argv[0]);
            return -1;
        }
        return 0;
    }
    else
    {
        return -2;
    }
}

void usage_rmdir(void)
{
    printf("rmdir <director>\n");
}
