#include "util.h"
#include "commands.h"
#include <sys/stat.h>
#include <sys/types.h>

#define PATH_TOKEN "/"
#define MAX_STACK_DEPTH 32
#define MAX_PATH_LENGTH 128

int search_command(const char *cmd)
{
    for (int i = 0; i < command_num; i++)
    {
        if (strcmp(cmd, cmd_list[i].cmd_str) == 0)
        {
            return i;
        }
    }

    return -1;
}

void get_realpath(const char *usr_path, char *result)
{
    char *stack[MAX_STACK_DEPTH];
    int index = 0;
    char fullpath[MAX_PATH_LENGTH];
    char *tok;
    int i;

    if (usr_path[0] == '/')
    {
        strncpy(fullpath, usr_path, sizeof(fullpath) - 1);
    }
    else
    {
        snprintf(fullpath, sizeof(fullpath) - 1, "%s/%s", current_dir + strlen(chroot_path), usr_path);
    }

    /* parsing */
    tok = strtok(fullpath, PATH_TOKEN);
    if (tok == NULL)
    {
        goto out;
    }

    do
    {
        if (strcmp(tok, ".") == 0 || strcmp(tok, "") == 0)
        {
            ; // skip
        }
        else if (strcmp(tok, "..") == 0)
        {
            if (index > 0)
                index--;
        }
        else
        {
            stack[index++] = tok;
        }
    } while ((tok = strtok(NULL, PATH_TOKEN)) && (index < MAX_STACK_DEPTH));

out:
    strcpy(result, chroot_path);

    // TODO: boundary check
    for (i = 0; i < index; i++)
    {
        strcat(result, "/");
        strcat(result, stack[i]);
    }
}

void init(char *command, char *current_dir)
{
    command = (char *)malloc(MAX_CMD_SIZE);
    if (command == NULL)
    {
        perror("malloc");
        exit(1);
    }

    current_dir = (char *)malloc(MAX_CMDLINE_SIZE);
    if (current_dir == NULL)
    {
        perror("malloc");
        free(command);
        exit(1);
    }

    if (chdir(chroot_path) < 0)
    {
        mkdir(chroot_path, 0755);
        chdir(chroot_path);
    }
}

void get_current_directory(char *buffer, unsigned int buffer_size) {
    if (getcwd(buffer, buffer_size) == NULL) {
        perror("getcwd");
    }
}

void print_path(const char *current_dir)
{
    if (strlen(current_dir) == strlen(chroot_path))
    {
        printf("/");
    }
    printf("%s & ", current_dir + strlen(chroot_path));
}