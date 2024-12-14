#include "commands.h"
#include "utils.h"

int delete_directory_recursive(const char *path)
{
    struct stat stat_buf;
    if (stat(path, &stat_buf) < 0)
    {
        perror("stat");
        return -1;
    }

    if (S_ISDIR(stat_buf.st_mode))
    {
        DIR *dir = opendir(path);
        if (!dir)
        {
            perror("opendir");
            return -1;
        }

        struct dirent *entry;
        char entry_path[MAX_PATH_SIZE];

        while ((entry = readdir(dir)) != NULL)
        {
            // Skip "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);

            // Recurse for subdirectories
            if (delete_directory_recursive(entry_path) < 0)
            {
                closedir(dir);
                return -1;
            }
        }

        closedir(dir);

        // Remove the directory itself
        if (rmdir(path) < 0)
        {
            perror("rmdir");
            return -1;
        }
    }
    else
    {
        // Remove file
        if (unlink(path) < 0)
        {
            perror("unlink");
            return -1;
        }
    }

    return 0;
}

int cmd_rm(int argc, char **argv)
{
    char real_path[MAX_PATH_SIZE];
    if (argc == 2)
    {
        // Single file removal
        get_realpath(argv[1], real_path);
        if (unlink(real_path) < 0)
        {
            perror(argv[0]);
            return -1;
        }
        return 0;
    }
    else if (argc == 3 && strcmp(argv[1], "-rf") == 0)
    {
        // Recursive directory removal
        get_realpath(argv[2], real_path);
        if (delete_directory_recursive(real_path) < 0)
        {
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
    printf("Usage:\n");
    printf("  rm <file>\n");
    printf("  rm -rf <directory>\n");
}
