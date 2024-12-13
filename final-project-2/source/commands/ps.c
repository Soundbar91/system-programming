#include "commands.h"
#include "utils.h"

#define MAX_CMD (256)
#define MAX_TTY (64)
#define MAX_STAT (1024)
#define MAX_TIME (32)
#define MAX_ENV (256)
#define MAX_INFO (1024)
#define MAX_LINK (512)
#define MAX_PROCESS_SIZE (1024)

typedef struct my_proc
{
    pid_t pid;
    pid_t ppid;
    char cmd[MAX_CMD];
    char tty[MAX_TTY];
    char stat[16];
    char time[MAX_TIME];
} my_proc;

int parse_ps_options(int argc, char **argv, int *show_all, int *tree_list)
{
    if (argc < 1)
    {
        return -1;
    }
    else if (argc == 1)
    {
        return 0;
    }

    char *options = argv[1];
    if (options[0] != '-')
    {
        return -1;
    }

    for (int i = 1; options[i] != '\0'; i++)
    {
        if (options[i] == 'e')
        {
            *show_all = 1;
        }
        else if (options[i] == 'f')
        {
            *tree_list = 1;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

int cmd_ps(int argc, char **argv)
{
    int show_all = 0, tree_list = 0;

    if (parse_ps_options(argc, argv, &show_all, &tree_list) < 0)
    {
        return -2;
    }

    DIR *proc_dir;
    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir /proc");
        return -1;
    }

    struct dirent *entry;
    FILE *cmd_file;
    my_proc my_procs[MAX_PROCESS_SIZE];
    char path[MAX_PATH_SIZE], cmd_line[MAX_CMD], stat_info[MAX_STAT], link_path[MAX_LINK];
    int proc_index = 0;

    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strspn(entry->d_name, "0123456789") == strlen(entry->d_name))
        {
            pid_t pid = atoi(entry->d_name);

            // command line information read
            snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);
            cmd_file = fopen(path, "r");
            if (cmd_file)
            {
                if (fgets(cmd_line, sizeof(cmd_line), cmd_file))
                {
                    memset(my_procs[proc_index].cmd, 0, sizeof(my_procs[proc_index].cmd));
                    size_t len = strlen(cmd_line);
                    for (size_t i = 0; i < len; i++)
                    {
                        if (cmd_line[i] == '\0')
                        {
                            cmd_line[i] = ' ';
                        }
                    }

                    strncpy(my_procs[proc_index].cmd, cmd_line, sizeof(my_procs[proc_index].cmd) - 1);
                    my_procs[proc_index].cmd[sizeof(my_procs[proc_index].cmd) - 1] = '\0';
                }

                fclose(cmd_file);
            }

            // stat information read
            snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
            cmd_file = fopen(path, "r");
            if (cmd_file)
            {
                if (fgets(stat_info, sizeof(stat_info), cmd_file))
                {
                    char *token = strtok(stat_info, " ");
                    int token_index = 0;
                    long total_time = 0;

                    while (token != NULL)
                    {
                        if (token_index == 2)
                        {
                            strncpy(my_procs[proc_index].stat, token, sizeof(my_procs[proc_index].stat));
                        }
                        else if (token_index == 3)
                        {
                            my_procs[proc_index].ppid = atoi(token);
                        }
                        else if (token_index == 13)
                        {
                            total_time += atol(token);
                        }
                        else if (token_index == 14)
                        {
                            total_time += atol(token);

                            long seconds = total_time / sysconf(_SC_CLK_TCK);
                            long hours = seconds / 3600;
                            long minutes = (seconds % 3600) / 60;
                            seconds = seconds % 60;

                            snprintf(my_procs[proc_index].time, sizeof(my_procs[proc_index].time), "%02ld:%02ld:%02ld", hours, minutes, seconds);
                        }

                        token = strtok(NULL, " ");
                        token_index++;
                    }
                    fclose(cmd_file);
                }
            }

            // tty information read
            snprintf(path, sizeof(path), "/proc/%s/fd/0", entry->d_name);
            size_t len = readlink(path, link_path, sizeof(link_path) - 1);
            if (len != -1)
            {
                link_path[len] = '\0';
                if (strncmp(link_path, "/dev", 4) == 0)
                {
                    strncpy(my_procs[proc_index].tty, link_path + 5, sizeof(my_procs[proc_index].tty) - 1);
                    my_procs[proc_index].tty[sizeof(my_procs[proc_index].tty) - 1] = '\0';
                }
                else
                {
                    strncpy(my_procs[proc_index].tty, "?", sizeof(my_procs[proc_index].tty) - 1);
                }
            }
            else
            {
                strncpy(my_procs[proc_index].tty, "?", sizeof(my_procs[proc_index].tty) - 1);
            }

            my_procs[proc_index].pid = pid;
            proc_index++;
            if (proc_index >= MAX_PROCESS_SIZE)
            {
                break;
            }
        }
    }
    closedir(proc_dir);

    char *tty = ttyname(STDIN_FILENO);
    char trimmed_tty[64] = "";

    if (tty && strstr(tty, "/dev/"))
    {
        snprintf(trimmed_tty, sizeof(trimmed_tty), "%s", tty + 5);
    }

    if (!show_all && !tree_list)
    {
        printf("%-8s %-8s %-10s %s\n", "PID", "TTY", "TIME", "CMD");
        for (int i = 0; i < proc_index; i++)
        {
            if (strcmp(my_procs[i].tty, trimmed_tty) == 0)
            {
                printf("%-8d %-8s %-10s %s\n",
                       my_procs[i].pid,
                       my_procs[i].tty,
                       my_procs[i].time,
                       my_procs[i].cmd);
            }
        }
    }
    else if (show_all)
    {
        // TODO. 옵션에 따른 명령어 출력 구현
        if (tree_list)
        {
            printf("pass\n");
        }
        else
        {
            printf("%-8s %-8s %-10s %s\n", "PID", "TTY", "TIME", "CMD");
            for (int i = 0; i < proc_index; i++)
            {
                printf("%-8d %-8s %-10s %s\n",
                       my_procs[i].pid,
                       my_procs[i].tty,
                       my_procs[i].time,
                       my_procs[i].cmd);
            }
        }
    }

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
}
