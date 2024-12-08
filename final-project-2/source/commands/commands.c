#include "commands.h"
#include "utils.h"

cmd_t cmd_list[] = {
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
    {"kill", cmd_kill, usage_kill, "terminate a process by PID"},
};

const int cmd_list_size = sizeof(cmd_list);