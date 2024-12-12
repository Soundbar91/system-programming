#include "commands.h"
#include "utils.h"
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define CURRENT_PATH "."

typedef struct file_info
{
    struct dirent dir_info;
    struct stat stat_info;
} file_info;

char *convert_size(off_t size)
{
    static char buffer[20];
    if (size < 1024)
    {
        snprintf(buffer, sizeof(buffer), "%ld B", size);
    }
    else if (size < 1024 * 1024)
    {
        snprintf(buffer, sizeof(buffer), "%.1f KB", size / 1024.0);
    }
    else if (size < 1024 * 1024 * 1024)
    {
        snprintf(buffer, sizeof(buffer), "%.1f MB", size / (1024.0 * 1024));
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%.1f GB", size / (1024.0 * 1024 * 1024));
    }
    return buffer;
}

char *get_permissions(mode_t mode)
{
    static char permissions[11];
    snprintf(permissions, sizeof(permissions), "%c%c%c%c%c%c%c%c%c%c",
             S_ISDIR(mode) ? 'd' : '-',
             mode & S_IRUSR ? 'r' : '-', mode & S_IWUSR ? 'w' : '-', mode & S_IXUSR ? 'x' : '-',
             mode & S_IRGRP ? 'r' : '-', mode & S_IWGRP ? 'w' : '-', mode & S_IXGRP ? 'x' : '-',
             mode & S_IROTH ? 'r' : '-', mode & S_IWOTH ? 'w' : '-',
             (mode & S_ISVTX) ? (mode & S_IXOTH ? 't' : 'T') : (mode & S_IXOTH ? 'x' : '-'));
    return permissions;
}

char *get_username(uid_t uid)
{
    struct passwd *pwd = getpwuid(uid);
    return pwd ? pwd->pw_name : "unknown";
}

char *get_groupname(gid_t gid)
{
    struct group *grp = getgrgid(gid);
    return grp ? grp->gr_name : "unknown";
}

char *get_time_string(time_t time)
{
    static char time_string[20];
    struct tm *tm_info = localtime(&time);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M", tm_info);
    return time_string;
}

int parse_options(int argc, char **argv, int *show_all, int *long_list, int *human_readable)
{
    if (argc == 1)
    {
        return 0;
    }

    char *options = argv[1];
    if (options[0] != '-')
    {
        return -1;
    }

    for (int i = 1; options[i] != '\0'; i++)
    {
        if (options[i] == 'a')
        {
            *show_all = 1;
        }
        else if (options[i] == 'l')
        {
            *long_list = 1;
        }
        else if (options[i] == 'h')
        {
            *human_readable = 1;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

void print_file_info(file_info *file, int human_readable)
{
    printf("%s %ld %s %s %5s %s %s\n",
           get_permissions(file->stat_info.st_mode),
           file->stat_info.st_nlink,
           get_username(file->stat_info.st_uid),
           get_groupname(file->stat_info.st_gid),
           human_readable ? convert_size(file->stat_info.st_size) : (char[]){file->stat_info.st_size, '\0'},
           get_time_string(file->stat_info.st_mtime),
           file->dir_info.d_name);
}

int cmd_ls(int argc, char **argv)
{
    int show_all = 0, long_list = 0, human_readable = 0;

    if (parse_options(argc, argv, &show_all, &long_list, &human_readable) < 0)
    {
        return -2;
    }

    DIR *dp = opendir(CURRENT_PATH);
    if (!dp)
    {
        perror("opendir error");
        return -1;
    }

    struct dirent *dep;
    while ((dep = readdir(dp)) != NULL)
    {
        if (!show_all && dep->d_name[0] == '.')
        {
            continue;
        }

        char full_path[MAX_PATH_SIZE];
        snprintf(full_path, sizeof(full_path), "%s/%s", CURRENT_PATH, dep->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) < 0)
        {
            perror("stat error");
            continue;
        }

        file_info file = {*dep, statbuf};
        if (long_list)
        {
            print_file_info(&file, human_readable);
        }
        else
        {
            printf("%s\n", dep->d_name);
        }
    }

    closedir(dp);
    return 0;
}

void usage_ls(void)
{
    printf("ls [options]\n");
    printf("Display the contents of the current directory.\n");
    printf("Options:\n");
    printf("  -l   Display detailed information (not implemented yet).\n");
    printf("Examples:\n");
    printf("  ls\n");
}
