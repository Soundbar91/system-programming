#include "commands.h"
#include "utils.h"

#define SYMBOLIC_REGEX "^[ugo]*[+-][rwx]+$"
#define NUMBER_REGEX "01234567"

int validate_symbolic_format(const char *symbolic)
{
    regex_t regex;

    if (regcomp(&regex, SYMBOLIC_REGEX, REG_EXTENDED) != 0)
    {
        perror("reg error");
        return -1;
    }

    int result = regexec(&regex, symbolic, 0, NULL, 0);
    regfree(&regex);

    return result == 0;
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
    char real_path[MAX_PATH_SIZE];
    struct stat buf;

    get_realpath(argv[2], real_path);

    if (strspn(argv[1], NUMBER_REGEX) == strlen(argv[1]))
    {
        if (chmod(real_path, strtol(argv[1], NULL, 8)) < 0)
        {
            perror("chmod error");
            return -1;
        }
        return 0;
    }
    else if (validate_symbolic_format(argv[1]) && strspn(argv[1], SYMBOLIC_REGEX) == strlen(argv[1]))
    {
        if (lstat(real_path, &buf) == -1)
        {
            perror("stat error");
            return -1;
        }

        mode_t mode = buf.st_mode;

        char *arg = argv[1];
        int i = 0;

        while (arg[i] != '\0')
        {
            char who = arg[i++];
            char op = arg[i++];

            while (arg[i] != '\0' && arg[i] != 'u' && arg[i] != 'g' && arg[i] != 'o')
            {
                char perms = arg[i++];
                update_permissions(&mode, who, op, perms);
            }
        }

        if (chmod(real_path, mode) < 0)
        {
            perror("chmod error");
            return -1;
        }

        return 0;
    }
    else
    {
        return -2;
    }
}

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
