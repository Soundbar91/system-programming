#include "commands.h"
#include "utils.h"
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_PATH_SIZE 128
#define MAX_PROCESS_SIZE 1024

/**
 * 2020136073 신관규
 * commands.c
 */

typedef struct my_proc
{
    pid_t pid;
    pid_t ppid;
    char cmd[256];
    char tty[64];
    char stat[16];
    char time[32];
    char env[265];
} my_proc;

cmd_t cmd_list[] = {
    {"help", cmd_help, usage_help, "show usage, ex) help <command>"},
    {"mkdir", cmd_mkdir, usage_mkdir, "create directory"},
    {"rmdir", cmd_rmdir, usage_rmdir, "remove directory"},
    {"cd", cmd_cd, usage_cd, "change current directory"},
    {"rename", cmd_rename, usage_rename, "rename directory & file"},
    {"ls", cmd_ls, NULL, "show directory contents"},
    {"quit", cmd_quit, usage_quit, "terminate shell"},
    {"ln", cmd_ln, usage_ln, "make hard link"},
    {"rm", cmd_rm, usage_rm, "remove file"},
    {"chmod", cmd_chmod, usage_chmod, "change file permissions (supports octal and symbolic modes)"},
    {"cat", cmd_cat, usage_cat, "display the contents of the specified file"},
    {"cp", cmd_cp, usage_cp, "copy the source file to the destination"},
    {"ps", cmd_ps, usage_ps, "show process list"},
    {"run", cmd_run, usage_run, "process programe"},
    {"kill", cmd_kill, usage_kill, "terminate a process by PID"},
};

const int command_num = sizeof(cmd_list) / sizeof(cmd_t);

int validate_path(const char *path)
{
    if (strncmp(path, chroot_path, strlen(chroot_path)) == 0)
    {
        if (path[strlen(chroot_path)] == '/' || path[strlen(chroot_path)] == '\0')
        {
            return 1;
        }
    }

    errno = EACCES;
    return 0;
}

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
        if (validate_path(rpath) == 0)
        {
            perror("chroot error");
            return -1;
        }
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
        if (validate_path(rpath) == 0)
        {
            perror("chroot error");
            return -1;
        }
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
        if (validate_path(rpath) == 0)
        {
            perror("chroot error");
            return -1;
        }
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

int cmd_rename(int argc, char **argv)
{
    char rpath1[128], rpath2[128];
    if (argc == 3)
    {
        get_realpath(argv[1], rpath1);
        get_realpath(argv[2], rpath2);
        if (validate_path(rpath1) == 0 || validate_path(rpath2) == 0)
        {
            perror("chroot error");
            return -1;
        }
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
    char link_target[512];
    ssize_t link_size;

    if ((dp = opendir(".")) == NULL)
    {
        perror("opendir error");
        return -1;
    }

    while ((dep = readdir(dp)) != NULL)
    {
        snprintf(full_path, sizeof(full_path), "./%s", dep->d_name);

        if (lstat(full_path, &buf) == -1)
        {
            perror("lstat error");
            continue;
        }

        print_permissions(buf.st_mode);
        print_file_metadata(&buf);

        printf("%s", dep->d_name);

        if (S_ISLNK(buf.st_mode))
        {
            link_size = readlink(full_path, link_target, sizeof(link_target) - 1);
            if (link_size != -1)
            {
                link_target[link_size] = '\0';
                printf(" -> %s", link_target);
            }
        }

        printf("\n");
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

    if (argc == 4 && strcmp(argv[1], "-s") == 0)
    {
        get_realpath(argv[2], rpath1);
        get_realpath(argv[3], rpath2);
        if (validate_path(rpath1) == 0 || validate_path(rpath2) == 0)
        {
            perror("chroot error");
            return -1;
        }
        if (symlink(rpath1, rpath2) < 0)
        {
            perror("symlink error");
            return -1;
        }
    }
    else if (argc == 3)
    {
        get_realpath(argv[1], rpath1);
        get_realpath(argv[2], rpath2);
        if (validate_path(rpath1) == 0 || validate_path(rpath2) == 0)
        {
            perror("chroot error");
            return -1;
        }
        if (link(rpath1, rpath2) < 0)
        {
            perror("link error");
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
        if (validate_path(rpath) == 0)
        {
            perror("chroot error");
            return -1;
        }
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

void update_permissions(mode_t *mode, char who, char op, char perms)
{
    mode_t mask = 0;

    if (perms == 'r')
    {
        mask = S_IRUSR | S_IRGRP | S_IROTH;
    }
    else if (perms == 'w')
    {
        mask = S_IWUSR | S_IWGRP | S_IWOTH;
    }
    else if (perms == 'x')
    {
        mask = S_IXUSR | S_IXGRP | S_IXOTH;
    }

    if (who == 'u')
    {
        mask &= S_IRWXU;
    }
    else if (who == 'g')
    {
        mask &= S_IRWXG;
    }
    else if (who == 'o')
    {
        mask &= S_IRWXO;
    }

    if (op == '+')
    {
        *mode |= mask;
    }
    else if (op == '-')
    {
        *mode &= ~mask;
    }
}

int cmd_chmod(int argc, char **argv)
{
    char rpath[128];
    struct stat buf;
    mode_t mode;

    get_realpath(argv[2], rpath);
    if (validate_path(rpath) == 0)
    {
        perror("chroot error");
        return -1;
    }

    if (strspn(argv[1], "01234567") == strlen(argv[1]))
    {
        if (chmod(rpath, strtol(argv[1], NULL, 8)) < 0)
        {
            perror("chmod error");
        }
    }
    else
    {
        if (lstat(rpath, &buf) == -1)
        {
            perror("stat error");
            return -1;
        }

        mode = buf.st_mode;
        for (int i = 0; argv[1][i] && argv[1][i + 1] && argv[1][i + 2]; i += 3)
        {
            update_permissions(&mode, argv[1][i], argv[1][i + 1], argv[1][i + 2]);
        }

        if (chmod(rpath, mode) < 0)
        {
            perror("chmod error");
        }
    }
    return 0;
}

int cmd_cat(int argc, char **argv)
{
    char rpath[128];
    FILE *file;
    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    if (argc != 2)
    {
        return -1;
    }

    get_realpath(argv[1], rpath);
    if (validate_path(rpath) == 0)
    {
        perror("chroot error");
        return -1;
    }

    file = fopen(rpath, "r");
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

int cmd_cp(int argc, char **argv)
{
    char rpath1[128];
    char rpath2[128];
    FILE *src = NULL, *dest = NULL;
    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    if (argc != 3)
    {
        return -1;
    }

    get_realpath(argv[1], rpath1);
    get_realpath(argv[2], rpath2);
    if (validate_path(rpath1) == 0 || validate_path(rpath2) == 0)
    {
        perror("chroot error");
        return -1;
    }

    src = fopen(rpath1, "r");
    if (src == NULL)
    {
        perror("fopen src error");
        return -1;
    }

    dest = fopen(rpath2, "w");
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

void print_tree(my_proc *procs, int proc_count, int pid, int depth)
{
    for (int i = 0; i < proc_count; i++)
    {
        if (procs[i].ppid == pid)
        {
            for (int j = 0; j < depth; j++)
            {
                printf("  ");
            }
            printf("%d\t%s\t%s\t%s\t%s\n",
                   procs[i].pid,
                   procs[i].tty,
                   procs[i].stat,
                   procs[i].time,
                   procs[i].cmd);

            print_tree(procs, proc_count, procs[i].pid, depth + 1);
        }
    }
}

int cmd_ps(int argc, char **argv)
{
    DIR *proc_dir;
    struct dirent *entry;
    char path[512], cmdline[256], stat_info[1024], link_path[512], env_info[100];
    FILE *cmd_file;
    my_proc my_procs[MAX_PROCESS_SIZE];
    int proc_index = 0;
    ssize_t len;

    if (argc < 1)
    {
        return -1;
    }

    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir /proc");
        return -1;
    }

    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strspn(entry->d_name, "0123456789") == strlen(entry->d_name))
        {
            pid_t pid = atoi(entry->d_name);

            snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);
            cmd_file = fopen(path, "r");
            if (cmd_file)
            {
                fgets(cmdline, sizeof(cmdline), cmd_file);
                fclose(cmd_file);
            }

            snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
            cmd_file = fopen(path, "r");
            if (cmd_file)
            {
                if (fgets(stat_info, sizeof(stat_info), cmd_file))
                {
                    char *token = strtok(stat_info, " ");
                    int token_index = 0;

                    while (token != NULL)
                    {
                        if (token_index == 1)
                        {
                            strncpy(my_procs[proc_index].cmd, token, sizeof(my_procs[proc_index].cmd));
                        }
                        else if (token_index == 2)
                        {
                            strncpy(my_procs[proc_index].stat, token, sizeof(my_procs[proc_index].stat));
                        }
                        else if (token_index == 3)
                        {
                            my_procs[proc_index].ppid = atoi(token);
                        }
                        else if (token_index == 13)
                        {
                            long utime = atol(token);
                            snprintf(my_procs[proc_index].time, sizeof(my_procs[proc_index].time), "%ld", utime);
                        }
                        token = strtok(NULL, " ");
                        token_index++;
                    }
                    fclose(cmd_file);
                }
            }

            snprintf(path, sizeof(path), "/proc/%s/fd/0", entry->d_name);
            len = readlink(path, link_path, sizeof(link_path) - 1);
            if (len != -1)
            {
                link_path[len] = '\0';
                if (strncmp(link_path, "/dev", 4) == 0)
                {
                    strncpy(my_procs[proc_index].tty, link_path + 5, sizeof(my_procs[proc_index].tty) - 1);
                    my_procs[proc_index].tty[sizeof(my_procs[proc_index].tty) - 1] = '\0';
                }
                else
                {
                    strncpy(my_procs[proc_index].tty, "?", sizeof(my_procs[proc_index].tty) - 1);
                }
            }
            else
            {
                strncpy(my_procs[proc_index].tty, "?", sizeof(my_procs[proc_index].tty) - 1);
            }

            snprintf(path, sizeof(path), "/proc/%s/environ", entry->d_name);
            cmd_file = fopen(path, "r");
            if (cmd_file)
            {
                size_t len = fread(env_info, 1, sizeof(env_info) - 1, cmd_file);
                env_info[len] = '\0';

                for (size_t i = 0; i < len; i++)
                {
                    if (env_info[i] == '\0')
                    {
                        env_info[i] = ' ';
                    }
                }

                strncpy(my_procs[proc_index].env, env_info, sizeof(my_procs[proc_index].env));
                fclose(cmd_file);
            }

            my_procs[proc_index].pid = pid;
            proc_index++;
            if (proc_index >= MAX_PROCESS_SIZE)
            {
                break;
            }
        }
    }
    closedir(proc_dir);

    if (argc == 1)
    {
        char *tty = ttyname(STDIN_FILENO);
        char trimmed_tty[64] = "";
        char *start = strstr(tty, "/dev/");
        start += 5;
        strncpy(trimmed_tty, start, sizeof(trimmed_tty) - 1);
        trimmed_tty[sizeof(trimmed_tty) - 1] = '\0';

        printf("PID\tTTY\tSTAT\tTIME\tCMD\n");
        for (int i = 0; i < proc_index; i++)
        {
            if (strcmp(my_procs[i].tty, trimmed_tty) == 0)
            {
                printf("%d\t%s\t%s\t%s\t%s\n",
                       my_procs[i].pid,
                       my_procs[i].tty,
                       my_procs[i].stat,
                       my_procs[i].time,
                       my_procs[i].cmd);
            }
        }
    }
    else
    {
        if (strcmp(argv[1], "e") == 0)
        {
            printf("PID\tTTY\tSTAT\tTIME\tCOMMEND\n");
            for (int i = 0; i < proc_index; i++)
            {
                if (strcmp(my_procs[i].tty, "?") != 0 && strcmp(my_procs[i].tty, "null") != 0)
                {
                    printf("%-5d\t%-5s\t%-5s\t%-5s\t%s%s\n",
                           my_procs[i].pid,
                           my_procs[i].tty,
                           my_procs[i].stat,
                           my_procs[i].time,
                           my_procs[i].cmd,
                           my_procs[i].env);
                }
            }
        }
        else if (strcmp(argv[1], "f") == 0)
        {
            printf("PID\tTTY\tSTAT\tTIME\tCMD\n");
            for (int i = 0; i < proc_index; i++)
            {
                if (my_procs[i].ppid == 0 || my_procs[i].ppid == 1)
                {
                    print_tree(my_procs, proc_index, my_procs[i].pid, 0);
                }
            }
        }
    }

    return 0;
}

int cmd_run(int argc, char **argv)
{
    if (argc < 2)
    {
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        if (execvp(argv[1], &argv[1]) < 0)
        {
            perror("execvp error");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("fork error");
        return -1;
    }

    return 0;
}

int cmd_kill(int argc, char **argv)
{
    if (argc != 2)
    {
        return -1;
    }

    pid_t pid = atoi(argv[1]);
    if (pid <= 0)
    {
        perror("invalid PID");
        return -1;
    }

    if (kill(pid, SIGKILL) == 0)
    {
        printf("Process %d killed successfully.\n", pid);
        return 0;
    }
    else
    {
        perror("kill error");
        return -1;
    }
}

void usage_help(void) { printf("help <command>\n"); }
void usage_mkdir(void) { printf("mkdir <directory>\n"); }
void usage_rmdir(void) { printf("rmdir <directory>\n"); }
void usage_cd(void) { printf("cd <directory>\n"); }
void usage_rename(void) { printf("rename <old_name> <new_name>\n"); }
void usage_quit(void) { printf("quit [n]\n"); }
void usage_ln(void) { printf("ln <original file> <new file>\n"); }
void usage_rm(void) { printf("rm <file>\n"); }
void usage_chmod(void)
{
    printf("chmod <mode> <file>\n");
    printf("Modes:\n");
    printf("  Symbolic mode: [who][+|-][permission]\n");
    printf("    who: u (user), g (group), o (others)\n");
    printf("    permission: r (read), w (write), x (execute)\n");
    printf("    Example: chmod u+rwx file.txt\n");
    printf("\n");
    printf("  Octal mode: a three-digit octal number\n");
    printf("    Example: chmod 755 file.txt\n");
}
void usage_cat(void) { printf("cat <filename>\n"); }
void usage_cp(void) { printf("cp <source> <destination>\n"); }
void usage_ps(void)
{
    printf("ps\n");
    printf("Displays a list of currently running processes.\n");
}
void usage_run(void)
{
    printf("run <program> [args...]\n");
    printf("Runs the specified program with optional arguments.\n");
}
void usage_kill(void)
{
    printf("kill <PID>\n");
    printf("Sends a SIGKILL signal to the specified process ID (PID).\n");
}
