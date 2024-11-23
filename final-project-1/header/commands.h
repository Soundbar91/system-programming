#ifndef COMMANDS_H
#define COMMANDS_H

#include "shell.h"

/**
 * 2020136073 신관규
 * commands.h
 */

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

#endif
