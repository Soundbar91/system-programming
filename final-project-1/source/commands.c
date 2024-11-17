#include "commands.h"
#include "utils.h"
#include <time.h>

#define BUFFER_SIZE 1024

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
    {"chmod", cmd_chmod, usage_chmod, "change file permissions (supports octal and symbolic modes)"},
    {"cat", cmd_cat, usage_cat, "display the contents of the specified file"},
    {"cp", cmd_cp, usage_cp, "copy the source file to the destination"},
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
    char link_target[512];
    ssize_t link_size;

    if ((dp = opendir(".")) == NULL) {
        perror("opendir error");
        return -1;
    }

    while ((dep = readdir(dp)) != NULL) {
        snprintf(full_path, sizeof(full_path), "./%s", dep->d_name);

        if (lstat(full_path, &buf) == -1) {
            perror("lstat error");
            continue;
        }

        print_permissions(buf.st_mode);
        print_file_metadata(&buf);

        printf("%s", dep->d_name);

        if (S_ISLNK(buf.st_mode)) {
            link_size = readlink(full_path, link_target, sizeof(link_target) - 1);
            if (link_size != -1) {
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

    if (argc == 4 && strcmp(argv[1], "-s") == 0) {
        get_realpath(argv[2], rpath1);
        get_realpath(argv[3], rpath2);

        if (symlink(rpath1, rpath2) < 0) {
            perror("symlink error");
            return -1;
        }
    } else if (argc == 3) {
        get_realpath(argv[1], rpath1);
        get_realpath(argv[2], rpath2);

        if (link(rpath1, rpath2) < 0) {
            perror("link error");
            return -1;
        }
    } else {
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
        fprintf(stderr, "Usage: cat <file>\n");
        return -1;
    }

    get_realpath(argv[1], rpath);

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

    if (argc != 3) {
        return -1;
    }

    get_realpath(argv[1], rpath1);
    get_realpath(argv[2], rpath2);

    src = fopen(rpath1, "r");
    if (src == NULL) {
        perror("fopen src error");
        return -1; 
    }

    dest = fopen(rpath2, "w");
    if (dest == NULL) {
        perror("fopen dest error");
        fclose(src);
        return -1;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytesRead, dest) < bytesRead) {
            perror("fwrite error");
            fclose(src);
            fclose(dest);
            return -1;
        }
    }

    if (ferror(src)) {
        perror("fread error");
        fclose(src);
        fclose(dest);
        return -1;
    }

    fclose(src);
    fclose(dest);
    return 0;
}

void usage_help(void) { printf("help <command>\n"); }
void usage_mkdir(void) { printf("mkdir <directory>\n"); }
void usage_rmdir(void) { printf("rmdir <directory>\n"); }
void usage_cd(void) { printf("cd <directory>\n"); }
void usage_mv(void) { printf("mv <old_name> <new_name>\n"); }
void usage_ln(void) { printf("ln <original file> <new file>\n"); }
void usage_rm(void) { printf("rm <file>\n"); }
void usage_chmod(void)
{
    printf("Usage: chmod <mode> <file>\n");
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