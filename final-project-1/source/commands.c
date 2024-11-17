#include "commands.h"
#include "utils.h"
#include <time.h>

cmd_t cmd_list[] = {
    {"help", cmd_help, usage_help, "show usage, ex) help <command>"},
    {"mkdir", cmd_mkdir, usage_mkdir, "create directory"},
    {"rmdir", cmd_rmdir, usage_rmdir, "remove directory"},
    {"cd", cmd_cd, usage_cd, "change current directory"},
    {"mv", cmd_mv, usage_mv, "rename directory & file"},
    {"ls", cmd_ls, NULL, "show directory contents"},
    {"quit", cmd_quit, NULL, "terminate shell"},
    {"ln", cmd_ln, usage_ln, "make hard link"},
    {"rm", cmd_rm, usage_rm, "remove file"},
};

const int command_num = sizeof(cmd_list) / sizeof(cmd_t);

int cmd_help(int argc, char **argv)
{
    int i;
    if (argc == 1)
    {
        for (i = 0; i < command_num; i++)
        {
            printf("%32s: %s\n", cmd_list[i].cmd_str, cmd_list[i].comment);
        }
    }
    else if (argv[1] != NULL)
    {
        i = search_command(argv[1]);
        if (i < 0)
        {
            printf("%s command not found\n", argv[1]);
        }
        else
        {
            if (cmd_list[i].usage_func)
            {
                printf("usage : ");
                cmd_list[i].usage_func();
                return 0;
            }
            else
            {
                printf("no usage\n");
                return -2;
            }
        }
    }
    return 0;
}

int cmd_mkdir(int argc, char **argv)
{
    char rpath[128];
    if (argc == 2)
    {
        get_realpath(argv[1], rpath);
        if (mkdir(rpath, 0755) < 0)
        {
            perror(argv[0]);
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

int cmd_rmdir(int argc, char **argv)
{
    char rpath[128];
    if (argc == 2)
    {
        get_realpath(argv[1], rpath);
        if (rmdir(rpath) < 0)
        {
            perror(argv[0]);
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

int cmd_cd(int argc, char **argv)
{
    char rpath[128];
    if (argc == 2)
    {
        get_realpath(argv[1], rpath);
        if (chdir(rpath) < 0)
        {
            perror(argv[0]);
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

int cmd_mv(int argc, char **argv)
{
    char rpath1[128], rpath2[128];
    if (argc == 3)
    {
        get_realpath(argv[1], rpath1);
        get_realpath(argv[2], rpath2);
        if (rename(rpath1, rpath2) < 0)
        {
            perror(argv[0]);
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

void print_permissions(mode_t mode)
{
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void print_file_metadata(const struct stat *buf)
{
    printf(" %2ld ", buf->st_nlink);

    printf("%d %d ", buf->st_uid, buf->st_gid);

    printf("%10ld ", buf->st_size);

    char time_buf[64];
    struct tm *tm_info = localtime(&buf->st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
    printf("%s ", time_buf);
}

int cmd_ls(int argc, char **argv)
{
    DIR *dp;
    struct dirent *dep;
    struct stat buf;
    char full_path[512];

    if ((dp = opendir(".")) == NULL)
    {
        perror("opendir error");
        return -1;
    }

    while ((dep = readdir(dp)) != NULL)
    {
        snprintf(full_path, sizeof(full_path), "./%s", dep->d_name);

        if (stat(full_path, &buf) == -1)
        {
            perror("stat error");
            continue;
        }

        print_permissions(buf.st_mode);
        print_file_metadata(&buf);
        printf("%s\n", dep->d_name);
    }

    closedir(dp);
    return 0;
}

int cmd_quit(int argc, char **argv)
{
    exit(0);
    return 0;
}

int cmd_ln(int argc, char **argv)
{
    char rpath1[128], rpath2[128];
    if (argc == 3)
    {
        get_realpath(argv[1], rpath1);
        get_realpath(argv[2], rpath2);
        if (link(rpath1, rpath2) < 0)
        {
            perror(argv[0]);
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

int cmd_rm(int argc, char **argv)
{
    char rpath[128];
    if (argc == 2)
    {
        get_realpath(argv[1], rpath);
        if (unlink(rpath) < 0)
        {
            perror(argv[0]);
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;   
}

void usage_help(void) { printf("help <command>\n"); }
void usage_mkdir(void) { printf("mkdir <directory>\n"); }
void usage_rmdir(void) { printf("rmdir <directory>\n"); }
void usage_cd(void) { printf("cd <directory>\n"); }
void usage_mv(void) { printf("mv <old_name> <new_name>\n"); }
void usage_ln(void) { printf("ln <original file> <new file>"); }
void usage_rm(void) { printf("rm <file>"); }