#include "commands.h"
#include "utils.h"

int cmd_rm(int argc, char **argv)
{
    char real_path[MAX_PATH_SIZE];
    if (argc == 2)
    {
        if (unlink(real_path) < 0)
        {
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

void usage_rm(void)
{
    printf("rm <file>\n");
}
