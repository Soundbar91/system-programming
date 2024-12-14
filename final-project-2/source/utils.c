#include "utils.h"
#include "shell.h"
#include "commands.h"
#include <signal.h>

#define MAX_INDEX_SIZE (32)
#define PATH_TOKEN "/"

int validate_path(const char *path)
{
    if (strncmp(path, chroot_path, strlen(chroot_path)) == 0)
    {
        if (path[strlen(chroot_path)] == '/' || path[strlen(chroot_path)] == '\0')
        {
            return 1;
        }
    }

    return 0;
}

int search_command(char *cmd)
{
    for (int i = 0; i < cmd_list_size; i++)
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
    char *stack[32];
    int index = 0;
    char fullpath[128];
    char *tok;
    int i;

    result[0] = '\0';

    if (usr_path[0] == '/')
    {
        strncpy(fullpath, usr_path, sizeof(fullpath) - 1);
    }
    else
    {
        snprintf(fullpath, sizeof(fullpath) - 1, "%s/%s", current_dir, usr_path);
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
    } while ((tok = strtok(NULL, PATH_TOKEN)) && (index < 32));

out:
    for (i = 0; i < index; i++)
    {
        strcat(result, "/");
        strcat(result, stack[i]);
    }
}

void setup_signal_handling()
{
    if (signal(SIGINT, SIG_IGN) == SIG_ERR)
    {
        perror("signal error");
        exit(1);
    }
}
