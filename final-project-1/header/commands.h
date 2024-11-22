#ifndef COMMANDS_H
#define COMMANDS_H

#include "shell.h"

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
DECLARE_CMDFUNC(ln);
DECLARE_CMDFUNC(rm);
DECLARE_CMDFUNC(chmod);
DECLARE_CMDFUNC(cat);
DECLARE_CMDFUNC(cp);
DECLARE_CMDFUNC(ps);
DECLARE_CMDFUNC(run);
DECLARE_CMDFUNC(kill);

int validate_and_get_realpath(const char *input_path, char *output_path);
int parse_proc_entry(pid_t pid, my_proc *proc);
void display_basic_ps(my_proc *procs, int count);
void display_ps_with_env(my_proc *procs, int count);
void display_process_tree(my_proc *procs, int count);
void print_tree(my_proc *procs, int count, int pid, int depth);

#endif
