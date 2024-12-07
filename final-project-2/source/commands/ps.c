#include "commands.h"
#include "utils.h"

int cmd_ps(int argc, char **argv) {
    /*기존 코드 리펙토링*/
    return 0;
}

void usage_ps(void)
{
    printf("ps [options]\n");
    printf("Displays a list of currently running processes.\n\n");
    printf("Options:\n");
    printf("  -e   Display all processes (including those not owned by the user).\n");
    printf("  -f   Display full format listing (detailed process information).\n\n");
    printf("Examples:\n");
    printf("  ps       Display processes owned by the user.\n");
    printf("  ps -e    Display all processes in the system.\n");
    printf("  ps -f    Display detailed information of processes owned by the user.\n");
    printf("  ps -e -f Display all processes with detailed information.\n");
}
