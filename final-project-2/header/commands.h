/**
 * 2020136073 신관규
 * commands.h
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <signal.h>
#include <dirent.h>

#define MAX_HELP_COMMENT (128)
#define MAX_PATH_SIZE (512)
#define MAX_CMD_SIZE (32)
#define MAX_BUFFER_SIZE (1024)
#define MAX_FILES (1024) 

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
extern cmd_t cmd_list[];
extern const int cmd_list_size;

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
DECLARE_CMDFUNC(kill);
DECLARE_CMDFUNC(mv);
