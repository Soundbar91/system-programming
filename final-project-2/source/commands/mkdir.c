#include "commands.h"
#include "utils.h"

#define MKDIR_MODE 0755

int cmd_mkdir(int argc, char **argv)
{
    char real_path[MAX_PATH_SIZE];
    if (argc == 2)
    {
        get_realpath(argv[1], real_path);
        if (mkdir(real_path, MKDIR_MODE) < 0) {
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

void usage_mkdir(void)
{
    printf("mkdir <director>\n");
}
