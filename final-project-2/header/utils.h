/**
 * 2020136073 신관규
 * utils.h
 */

int validate_path(const char *path);
int search_command(char *cmd);
void get_realpath(const char *usr_path, char *real_path);
void setup_signal_handling();
