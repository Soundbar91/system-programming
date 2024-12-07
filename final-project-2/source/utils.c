#include "utils.h"
#include "shell.h"
#include "commands.h"
#include <signal.h>

#define MAX_INDEX_SIZE (32)
#define PATH_TOKEN "/"

int search_command(char *cmd)
{
    for (int i = 0; i < sizeof(cmd_list); i++)
    {
        if (strcmp(cmd, cmd_list[i].cmd_str) == 0)
        {
            return i;
        }
    }

    return -1;
}

void get_realpath(const char *usr_path, char *real_path)
{
    char *stack[MAX_INDEX_SIZE];
    char fullpath[MAX_PATH_SIZE];
    int index = 0;
    char *tok;

    real_path[0] = '\0';

    if (usr_path[0] == '/')
    {
        strncpy(fullpath, usr_path, sizeof(fullpath) - 1);
    }
    else
    {
        snprintf(fullpath, sizeof(fullpath) - 1, "%s/%s", current_dir, usr_path);
    }

    tok = strtok(fullpath, PATH_TOKEN);
    if (tok == NULL)
    {
        goto out;
    }

    do
    {
        if (strcmp(tok, "..") == 0)
        {
            if (index > 0)
            {
                index--;
            }
        }
        else
        {
            stack[index++] = tok;
        }
    } while ((tok = strtok(NULL, PATH_TOKEN)) && (index < MAX_INDEX_SIZE));

out:
    for (int i = 0; i < index; i++)
    {
        strcat(real_path, PATH_TOKEN);
        strcat(real_path, stack[i]);
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
