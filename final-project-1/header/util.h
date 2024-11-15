#ifndef UTIL_H
#define UTIL_H

int search_command(const char *cmd);
void get_realpath(const char *usr_path, char *result);
void init(char *command, char *current_dir);
void get_current_directory(char *buffer, unsigned int buffer_size);
void print_path(const char *current_dir);

#endif
