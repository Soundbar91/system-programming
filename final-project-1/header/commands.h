#ifndef COMMANDS_H
#define COMMANDS_H

#include "shell.h"

#define DECLARE_CMDFUNC(str) int cmd_##str(int argc, char **argv); \
                             void usage_##str(void)

DECLARE_CMDFUNC(help);
DECLARE_CMDFUNC(mkdir);
DECLARE_CMDFUNC(rmdir);
DECLARE_CMDFUNC(cd);
DECLARE_CMDFUNC(mv);
DECLARE_CMDFUNC(ls);
DECLARE_CMDFUNC(quit);

#endif
