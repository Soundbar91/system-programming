/**
 * 2020136073 신관규
 * commands.h
 */

#define MAX_HELP_COMMENT (128)
#define MAX_CMD_SIZE (32)

typedef int (*cmd_func_t)(int argc, char **argv);
typedef void (*usage_func_t)(void);

typedef struct cmd_t
{
    char cmd_str[MAX_CMD_SIZE];
    cmd_func_t cmd_func;
    usage_func_t usage_func;
    char comment[MAX_HELP_COMMENT];
} cmd_t;

extern cmd_t cmd_list[] = {
    {"help", cmd_help, usage_help, "show usage, ex) help <command>"},
};

#define DECLARE_CMDFUNC(str)              \
    int cmd_##str(int argc, char **argv); \
    void usage_##str(void)

DECLARE_CMDFUNC(help);