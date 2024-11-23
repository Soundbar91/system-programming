#ifndef UTILS_H
#define UTILS_H

#include "shell.h"

/**
 * 2020136073 신관규
 * utils.h
 */

int search_command(char *cmd);
void get_realpath(const char *usr_path, char *result);
const char *get_type_str(char type);

#endif
