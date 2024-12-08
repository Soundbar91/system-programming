#include "commands.h"
#include "utils.h"

int cmd_cp(int argc, char **argv)
{
    char src_path[MAX_PATH_SIZE];
    char dest_path[MAX_PATH_SIZE];
    FILE *src, *dest;
    char buffer[MAX_BUFFER_SIZE];
    size_t bytesRead;

    if (argc != 3)
    {
        return -2;
    }

    get_realpath(argv[1], src_path);
    get_realpath(argv[2], dest_path);

    src = fopen(src_path, "r");
    if (src == NULL)
    {
        perror("fopen src error");
        return -1;
    }

    dest = fopen(dest_path, "w");
    if (dest == NULL)
    {
        perror("fopen dest error");
        fclose(src);
        return -1;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, dest) < bytesRead)
        {
            perror("fwrite error");
            fclose(src);
            fclose(dest);
            return -1;
        }
    }

    if (ferror(src))
    {
        perror("fread error");
        fclose(src);
        fclose(dest);
        return -1;
    }

    fclose(src);
    fclose(dest);
    return 0;
}

void usage_cp(void)
{
    printf("cp <source> <destination>\n");
}
