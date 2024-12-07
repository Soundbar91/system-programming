/**
 * 2020136073 신관규
 * commands.h
 */

#include <string.h>

#define MAX_HELP_COMMENT (128)
#define MAX_PATH_SIZE (128)
#define MAX_CMD_SIZE (32)

// function structure
typedef int (*cmd_func_t)(int argc, char **argv);
typedef void (*usage_func_t)(void);

typedef struct cmd_t
{
    char cmd_str[MAX_CMD_SIZE];
    cmd_func_t cmd_func;
    usage_func_t usage_func;
    char comment[MAX_HELP_COMMENT];
} cmd_t;

// function list
extern cmd_t cmd_list[] = {
    {"help", cmd_help, usage_help, "show usage, ex) help <command>"},
    {"mkdir", cmd_mkdir, usage_mkdir, "create directory"},
    {"rmdir", cmd_rmdir, usage_rmdir, "remove directory"},
    {"cd", cmd_cd, usage_cd, "change current directory"},
    {"rename", cmd_rename, usage_rename, "rename directory & file"},
    {"ls", cmd_ls, usage_ls, "show directory contents"},
    {"quit", cmd_quit, usage_quit, "terminate shell"},
    {"rm", cmd_rm, usage_rm, "remove file"},
    {"chmod", cmd_chmod, usage_chmod, "change file permissions"},
    {"cat", cmd_cat, usage_cat, "display the contents of the specified file"},
    {"cp", cmd_cp, usage_cp, "copy the source file to the destination"},
    {"ps", cmd_ps, usage_ps, "show process list"},
    {"run", cmd_run, usage_run, "process programe"},
};

// function macro
#define DECLARE_CMDFUNC(str)              \
    int cmd_##str(int argc, char **argv); \
    void usage_##str(void)

DECLARE_CMDFUNC(help);
DECLARE_CMDFUNC(mkdir);
DECLARE_CMDFUNC(rmdir);
DECLARE_CMDFUNC(cd);
DECLARE_CMDFUNC(rename);
DECLARE_CMDFUNC(ls);
DECLARE_CMDFUNC(quit);
DECLARE_CMDFUNC(rm);
DECLARE_CMDFUNC(chmod);
DECLARE_CMDFUNC(cat);
DECLARE_CMDFUNC(cp);
DECLARE_CMDFUNC(ps);
DECLARE_CMDFUNC(run);
