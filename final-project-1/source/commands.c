#include "commands.h"
#include "util.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

cmd_t cmd_list[] = {
    {"help", cmd_help, usage_help, "Show available commands"},
    {"mkdir", cmd_mkdir, usage_mkdir, "Create a directory"},
    {"rmdir", cmd_rmdir, usage_rmdir, "Remove a directory"},
    {"cd", cmd_cd, usage_cd, "Change directory"},
    {"mv", cmd_mv, usage_mv, "Move or rename a file"},
    {"ls", cmd_ls, NULL, "List directory contents"},
    {"quit", cmd_quit, NULL, "Quit the shell"},
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
                return (0);
            }
            else
            {
                printf("no usage\n");
                return (-2);
            }
        }
    }

    return (0);
}

int cmd_mkdir(int argc, char **argv)
{
    int ret = 0;
    char rpath[128];

    if (argc == 2)
    {
        get_realpath(argv[1], rpath);

        if ((ret = mkdir(rpath, 0755)) < 0)
        {
            perror(argv[0]);
        }
    }
    else
    {
        ret = -2; // syntax error
    }

    return (ret);
}

int cmd_rmdir(int argc, char **argv)
{
    int ret = 0;
    char rpath[128];

    if (argc == 2)
    {
        get_realpath(argv[1], rpath);

        if ((ret = rmdir(rpath)) < 0)
        {
            perror(argv[0]);
        }
    }
    else
    {
        ret = -2; // syntax error
    }

    return (ret);
}

int cmd_cd(int argc, char **argv)
{
    int ret = 0;
    char rpath[128];

    if (argc == 2)
    {
        get_realpath(argv[1], rpath);

        if ((ret = chdir(rpath)) < 0)
        {
            perror(argv[0]);
        }
    }
    else
    {
        ret = -2;
    }

    return (ret);
}

int cmd_mv(int argc, char **argv)
{
    int ret = 0;
    char rpath1[128];
    char rpath2[128];

    if (argc == 3)
    {

        get_realpath(argv[1], rpath1);
        get_realpath(argv[2], rpath2);

        if ((ret = rename(rpath1, rpath2)) < 0)
        {
            perror(argv[0]);
        }
    }
    else
    {
        ret = -2;
    }

    return (ret);
}

static const char *get_type_str(char type)
{
    switch (type)
    {
    case DT_BLK:
        return "BLK";
    case DT_CHR:
        return "CHR";
    case DT_DIR:
        return "DIR";
    case DT_FIFO:
        return "FIFO";
    case DT_LNK:
        return "LNK";
    case DT_REG:
        return "REG";
    case DT_SOCK:
        return "SOCK";
    default: // include DT_UNKNOWN
        return "UNKN";
    }
}

int cmd_ls(int argc, char **argv)
{
    int ret = 0;
    DIR *dp;
    struct dirent *dep;

    if (argc != 1)
    {
        ret = -2;
        goto out;
    }

    if ((dp = opendir(".")) == NULL)
    {
        ret = -1;
        goto out;
    }

    while ((dep = readdir(dp)))
    {
        printf("%10ld %4s %s\n", dep->d_ino, get_type_str(dep->d_type), dep->d_name);
    }

    closedir(dp);

out:
    return (ret);
}

int cmd_quit(int argc, char **argv)
{
    exit(1);
    return 0;
}

void usage_help(void)
{
    printf("help <command>\n");
}

void usage_mkdir(void)
{
    printf("mkdir <directory>\n");
}

void usage_rmdir(void)
{
    printf("rmdir <directory>\n");
}

void usage_cd(void)
{
    printf("cd <directory>\n");
}

void usage_mv(void)
{
    printf("mv <old_name> <new_name>\n");
}
