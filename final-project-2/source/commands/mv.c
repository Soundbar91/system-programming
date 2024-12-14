#include "commands.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int copy_file(const char *src, const char *dest)
{
    FILE *src_file = fopen(src, "rb");
    if (!src_file)
    {
        perror("fopen(src)");
        return -1;
    }

    FILE *dest_file = fopen(dest, "wb");
    if (!dest_file)
    {
        perror("fopen(dest)");
        fclose(src_file);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0)
    {
        if (fwrite(buffer, 1, bytes, dest_file) != bytes)
        {
            perror("fwrite");
            fclose(src_file);
            fclose(dest_file);
            return -1;
        }
    }

    fclose(src_file);
    fclose(dest_file);
    return 0;
}

int cmd_mv(int argc, char **argv)
{
    char real_path_old[MAX_PATH_SIZE], real_path_new[MAX_PATH_SIZE];
    struct stat stat_buf;

    if (argc != 3)
    {
        return -2;
    }

    get_realpath(argv[1], real_path_old);
    get_realpath(argv[2], real_path_new);

    if (stat(real_path_old, &stat_buf) < 0)
    {
        perror("stat");
        return -1;
    }

    if (rename(real_path_old, real_path_new) == 0)
    {
        return 0;
    }

    if (S_ISREG(stat_buf.st_mode))
    {
        if (copy_file(real_path_old, real_path_new) < 0)
        {
            return -1;
        }
    }
    else if (S_ISDIR(stat_buf.st_mode))
    {
        fprintf(stderr, "Directory move not supported yet.\n");
        return -1;
    }
    else
    {
        fprintf(stderr, "Unsupported file type.\n");
        return -1;
    }

    if (unlink(real_path_old) < 0)
    {
        perror("unlink");
        return -1;
    }

    return 0;
}

void usage_mv(void) 
{
    printf("Usage: mv <source> <destination>\n"); 
}
