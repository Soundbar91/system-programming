#ifndef UTILS_H
#define UTILS_H

#include "shell.h"

int search_command(char *cmd);
void get_realpath(char *usr_path, char *result);
const char *get_type_str(char type);

#endif
