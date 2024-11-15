#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_CMDLINE_SIZE    (128)
#define MAX_CMD_SIZE        (32)
#define MAX_ARG             (4)

extern char *current_dir;
extern char *chroot_path;

typedef int (*cmd_func_t)(int argc, char **argv);
typedef void (*usage_func_t)(void);

typedef struct cmd_t {
    char cmd_str[MAX_CMD_SIZE];
    cmd_func_t cmd_func;
    usage_func_t usage_func;
    char comment[128];
} cmd_t;

extern cmd_t cmd_list[];
extern const int command_num;

#endif
