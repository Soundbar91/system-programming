#include "commands.h"
#include "utils.h"

cmd_t cmd_list[] = {
    {"help",    cmd_help,    usage_help,  "show usage, ex) help <command>"},
    {"mkdir",   cmd_mkdir,   usage_mkdir, "create directory"},
    {"rmdir",   cmd_rmdir,   usage_rmdir, "remove directory"},
    {"cd",      cmd_cd,      usage_cd,    "change current directory"},
    {"mv",      cmd_mv,      usage_mv,    "rename directory & file"},
    {"ls",      cmd_ls,      NULL,        "show directory contents"},
    {"quit",    cmd_quit,    NULL,        "terminate shell"},
};

const int command_num = sizeof(cmd_list) / sizeof(cmd_t);

int cmd_help(int argc, char **argv)
{
    int i;
    if (argc == 1) {
        for (i = 0; i < command_num; i++) {
            printf("%32s: %s\n", cmd_list[i].cmd_str, cmd_list[i].comment);
        }
    } else if (argv[1] != NULL) {
        i = search_command(argv[1]);
        if (i < 0) {
            printf("%s command not found\n", argv[1]);
        } else {
            if (cmd_list[i].usage_func) {
                printf("usage : ");
                cmd_list[i].usage_func();
                return 0;
            } else {
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
    if (argc == 2) {
        get_realpath(argv[1], rpath);
        if (mkdir(rpath, 0755) < 0) {
            perror(argv[0]);
            return -1;
        }
    } else {
        return -2;
    }
    return 0;
}

int cmd_rmdir(int argc, char **argv)
{
    char rpath[128];
    if (argc == 2) {
        get_realpath(argv[1], rpath);
        if (rmdir(rpath) < 0) {
            perror(argv[0]);
            return -1;
        }
    } else {
        return -2;
    }
    return 0;
}

int cmd_cd(int argc, char **argv)
{
    char rpath[128];
    if (argc == 2) {
        get_realpath(argv[1], rpath);
        if (chdir(rpath) < 0) {
            perror(argv[0]);
            return -1;
        }
    } else {
        return -2;
    }
    return 0;
}

int cmd_mv(int argc, char **argv)
{
    char rpath1[128], rpath2[128];
    if (argc == 3) {
        get_realpath(argv[1], rpath1);
        get_realpath(argv[2], rpath2);
        if (rename(rpath1, rpath2) < 0) {
            perror(argv[0]);
            return -1;
        }
    } else {
        return -2;
    }
    return 0;
}

int cmd_ls(int argc, char **argv)
{
    DIR *dp;
    struct dirent *dep;
    if (argc != 1) return -2;
    if ((dp = opendir(".")) == NULL) return -1;
    while ((dep = readdir(dp))) {
        printf("%10ld %4s %s\n", dep->d_ino, get_type_str(dep->d_type), dep->d_name);
    }
    closedir(dp);
    return 0;
}

int cmd_quit(int argc, char **argv)
{
    exit(0);
    return 0;
}

void usage_help(void) { printf("help <command>\n"); }
void usage_mkdir(void) { printf("mkdir <directory>\n"); }
void usage_rmdir(void) { printf("rmdir <directory>\n"); }
void usage_cd(void) { printf("cd <directory>\n"); }
void usage_mv(void) { printf("mv <old_name> <new_name>\n"); }
