#include "commands.h"
#include "utils.h"

int cmd_rename(int argc, char **argv)
{
    char real_path_old[MAX_PATH_SIZE], real_path_new[MAX_PATH_SIZE];
    if (argc == 3)
    {
        get_realpath(argv[1], real_path_old);
        get_realpath(argv[2], real_path_new);
        if (rename(real_path_old, real_path_new) < 0)
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

void usage_rename(void) 
{
    printf("rename <old_name> <new_name>%n"); 
}
