#include "utils.h"

#define PATH_TOKEN "/"

/**
 * 2020136073 신관규
 * utils.c
 */

int search_command(char *cmd)
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

const char *get_type_str(char type)
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
    default:
        return "UNKN";
    }
}
