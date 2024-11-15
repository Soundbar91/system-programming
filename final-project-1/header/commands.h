#ifndef COMMANDS_H
#define COMMANDS_H

#include "config.h"

int cmd_help(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_rmdir(int argc, char **argv);
int cmd_cd(int argc, char **argv);
int cmd_mv(int argc, char **argv);
int cmd_ls(int argc, char **argv);
int cmd_quit(int argc, char **argv);

void usage_help(void);
void usage_mkdir(void);
void usage_rmdir(void);
void usage_cd(void);
void usage_mv(void);

#endif
