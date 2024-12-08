#include "commands.h"
#include "utils.h"

#define MAX_BUFFER_SIZE 1024

int cmd_cat(int argc, char **argv)
{
    char real_path[MAX_PATH_SIZE];
    FILE *file;
    char buffer[MAX_BUFFER_SIZE];
    size_t bytesRead;

    if (argc != 2)
    {
        return -2;
    }

    get_realpath(argv[1], real_path);

    file = fopen(real_path, "r");
    if (file == NULL)
    {
        perror("fopen error");
        return -1;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, stdout) < bytesRead)
        {
            perror("fwrite error");
            fclose(file);
            return -1;
        }
    }

    if (ferror(file))
    {
        perror("fread error");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

void usage_cat(void)
{
    printf("cat <filename>\n");
}