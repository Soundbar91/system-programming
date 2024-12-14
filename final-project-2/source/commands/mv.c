#include "commands.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h> // For basename()

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

int copy_directory(const char *src, const char *dest)
{
    struct stat stat_buf;
    if (stat(src, &stat_buf) < 0)
    {
        perror("stat");
        return -1;
    }

    if (mkdir(dest, stat_buf.st_mode) < 0 && errno != EEXIST)
    {
        perror("mkdir");
        return -1;
    }

    DIR *dir = opendir(src);
    if (!dir)
    {
        perror("opendir");
        return -1;
    }

    struct dirent *entry;
    char src_path[MAX_PATH_SIZE], dest_path[MAX_PATH_SIZE];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        if (stat(src_path, &stat_buf) < 0)
        {
            perror("stat");
            closedir(dir);
            return -1;
        }

        if (S_ISDIR(stat_buf.st_mode))
        {
            if (copy_directory(src_path, dest_path) < 0)
            {
                closedir(dir);
                return -1;
            }
        }
        else if (S_ISREG(stat_buf.st_mode))
        {
            if (copy_file(src_path, dest_path) < 0)
            {
                closedir(dir);
                return -1;
            }
        }
        else
        {
            fprintf(stderr, "Unsupported file type: %s\n", src_path);
        }
    }

    closedir(dir);
    return 0;
}

int delete_directory(const char *path)
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
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);

        struct stat stat_buf;
        if (stat(entry_path, &stat_buf) < 0)
        {
            perror("stat");
            closedir(dir);
            return -1;
        }

        if (S_ISDIR(stat_buf.st_mode))
        {
            if (delete_directory(entry_path) < 0)
            {
                closedir(dir);
                return -1;
            }
        }
        else
        {
            if (unlink(entry_path) < 0)
            {
                perror("unlink");
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);

    if (rmdir(path) < 0)
    {
        perror("rmdir");
        return -1;
    }

    return 0;
}

int cmd_mv(int argc, char **argv)
{
    char real_path_old[MAX_PATH_SIZE], real_path_new[MAX_PATH_SIZE];
    struct stat src_stat, dest_stat;

    if (argc != 3)
    {
        fprintf(stderr, "Error: Invalid arguments\n");
        usage_mv();
        return -2;
    }

    // Resolve source and destination paths
    get_realpath(argv[1], real_path_old);
    get_realpath(argv[2], real_path_new);

    if (stat(real_path_old, &src_stat) < 0)
    {
        perror("stat(src)");
        return -1;
    }

    // Check if destination is a directory
    if (stat(real_path_new, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode))
    {
        char *file_name = basename(real_path_old);
        snprintf(real_path_new, sizeof(real_path_new), "%s/%s", argv[2], file_name);
    }

    // Try rename first
    if (rename(real_path_old, real_path_new) == 0)
    {
        return 0;
    }

    // If rename fails, handle copy and delete
    if (S_ISREG(src_stat.st_mode))
    {
        if (copy_file(real_path_old, real_path_new) < 0)
        {
            return -1;
        }

        if (unlink(real_path_old) < 0)
        {
            perror("unlink");
            return -1;
        }
    }
    else if (S_ISDIR(src_stat.st_mode))
    {
        if (copy_directory(real_path_old, real_path_new) < 0)
        {
            return -1;
        }

        if (delete_directory(real_path_old) < 0)
        {
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Unsupported file type: %s\n", real_path_old);
        return -1;
    }

    return 0;
}

void usage_mv(void)
{
    printf("Usage: mv <source> <destination>\n");
}
