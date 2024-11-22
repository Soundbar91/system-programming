#include "commands.h"
#include "utils.h"
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_PATH_SIZE 128
#define MAX_PROCESS_SIZE 1024

typedef struct my_proc {
    pid_t pid;
    pid_t ppid;
    char cmd[256];
    char tty[64];
    char stat[16];
    char time[32];
    char env[265];
} my_proc;

cmd_t cmd_list[] = {
    {"help", cmd_help, usage_help, "Show command usage, e.g., help <command>"},
    {"mkdir", cmd_mkdir, usage_mkdir, "Create a directory"},
    {"rmdir", cmd_rmdir, usage_rmdir, "Remove a directory"},
    {"cd", cmd_cd, usage_cd, "Change the current directory"},
    {"rename", cmd_rename, usage_rename, "Rename files or directories"},
    {"ls", cmd_ls, NULL, "List directory contents"},
    {"quit", cmd_quit, usage_quit, "Exit the shell"},
    {"ln", cmd_ln, usage_ln, "Create a link (hard or symbolic)"},
    {"rm", cmd_rm, usage_rm, "Remove a file"},
    {"chmod", cmd_chmod, usage_chmod, "Change file permissions"},
    {"cat", cmd_cat, usage_cat, "Display file contents"},
    {"cp", cmd_cp, usage_cp, "Copy files"},
    {"ps", cmd_ps, usage_ps, "Show process list"},
    {"run", cmd_run, usage_run, "Run a program"},
    {"kill", cmd_kill, usage_kill, "Kill a process by PID"},
};

const int command_num = sizeof(cmd_list) / sizeof(cmd_t);

int validate_and_get_realpath(const char *input_path, char *output_path) {
    get_realpath(input_path, output_path);
    if (validate_path(output_path) == 0) {
        perror("Path validation failed");
        return -1;
    }
    return 0;
}

int cmd_mkdir(int argc, char **argv) {
    if (argc != 2) return -2;

    char rpath[MAX_PATH_SIZE];
    if (validate_and_get_realpath(argv[1], rpath) < 0) return -1;

    if (mkdir(rpath, 0755) < 0) {
        perror("mkdir error");
        return -1;
    }
    return 0;
}

int cmd_rmdir(int argc, char **argv) {
    if (argc != 2) return -2;

    char rpath[MAX_PATH_SIZE];
    if (validate_and_get_realpath(argv[1], rpath) < 0) return -1;

    if (rmdir(rpath) < 0) {
        perror("rmdir error");
        return -1;
    }
    return 0;
}

int cmd_cd(int argc, char **argv) {
    if (argc != 2) return -2;

    char rpath[MAX_PATH_SIZE];
    if (validate_and_get_realpath(argv[1], rpath) < 0) return -1;

    if (chdir(rpath) < 0) {
        perror("chdir error");
        return -1;
    }
    return 0;
}

int cmd_chmod(int argc, char **argv) {
    if (argc != 3) return -2;

    char rpath[MAX_PATH_SIZE];
    struct stat buf;
    if (validate_and_get_realpath(argv[2], rpath) < 0) return -1;

    if (stat(rpath, &buf) < 0) {
        perror("stat error");
        return -1;
    }

    if (strspn(argv[1], "01234567") == strlen(argv[1])) {
        mode_t new_mode = strtol(argv[1], NULL, 8);
        if (chmod(rpath, new_mode) < 0) {
            perror("chmod error");
            return -1;
        }
    } else {
        fprintf(stderr, "Unsupported mode format\n");
        return -1;
    }
    return 0;
}

int cmd_cat(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: cat <file>\n");
        return -2;
    }

    char rpath[MAX_PATH_SIZE];
    if (validate_and_get_realpath(argv[1], rpath) < 0) return -1;

    FILE *file = fopen(rpath, "r");
    if (!file) {
        perror("fopen error");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        fwrite(buffer, 1, bytesRead, stdout);
    }
    if (ferror(file)) {
        perror("fread error");
    }

    fclose(file);
    return 0;
}

int cmd_ps(int argc, char **argv) {
    DIR *proc_dir;
    struct dirent *entry;
    my_proc my_procs[MAX_PROCESS_SIZE];
    int proc_index = 0;

    proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("opendir error");
        return -1;
    }

    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR && strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            pid_t pid = atoi(entry->d_name);
            my_proc proc;

            if (parse_proc_entry(pid, &proc) == 0) {
                my_procs[proc_index++] = proc;
                if (proc_index >= MAX_PROCESS_SIZE) break;
            }
        }
    }
    closedir(proc_dir);

    if (argc == 1) {
        display_basic_ps(my_procs, proc_index);
    } else if (strcmp(argv[1], "e") == 0) {
        display_ps_with_env(my_procs, proc_index);
    } else if (strcmp(argv[1], "f") == 0) {
        display_process_tree(my_procs, proc_index);
    } else {
        fprintf(stderr, "Invalid option for ps. Available options: [none], e, f\n");
        return -1;
    }

    return 0;
}

int parse_proc_entry(pid_t pid, my_proc *proc) {
    char path[512], buffer[256];
    FILE *file;

    memset(proc, 0, sizeof(my_proc));
    proc->pid = pid;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (!file) return -1;

    if (fgets(buffer, sizeof(buffer), file)) {
        sscanf(buffer, "%d %*s %s %d", &proc->pid, proc->stat, &proc->ppid);
    }
    fclose(file);

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    file = fopen(path, "r");
    if (file) {
        if (fgets(buffer, sizeof(buffer), file)) {
            strncpy(proc->cmd, buffer, sizeof(proc->cmd) - 1);
        }
        fclose(file);
    } else {
        strncpy(proc->cmd, "[unknown]", sizeof(proc->cmd) - 1);
    }

    snprintf(path, sizeof(path), "/proc/%d/fd/0", pid);
    ssize_t len = readlink(path, buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        if (strncmp(buffer, "/dev", 4) == 0) {
            strncpy(proc->tty, buffer + 5, sizeof(proc->tty) - 1);
        } else {
            strncpy(proc->tty, "?", sizeof(proc->tty) - 1);
        }
    } else {
        strncpy(proc->tty, "?", sizeof(proc->tty) - 1);
    }

    return 0;
}

void display_basic_ps(my_proc *procs, int count) {
    char *current_tty = ttyname(STDIN_FILENO);
    char tty_trimmed[64] = "";
    if (current_tty) {
        strncpy(tty_trimmed, strstr(current_tty, "/dev/") + 5, sizeof(tty_trimmed) - 1);
    }

    printf("PID\tTTY\tSTAT\tCMD\n");
    for (int i = 0; i < count; i++) {
        if (strcmp(procs[i].tty, tty_trimmed) == 0) {
            printf("%d\t%s\t%s\t%s\n", procs[i].pid, procs[i].tty, procs[i].stat, procs[i].cmd);
        }
    }
}

void display_ps_with_env(my_proc *procs, int count) {
    printf("PID\tTTY\tSTAT\tCMD\tENV\n");
    for (int i = 0; i < count; i++) {
        printf("%d\t%s\t%s\t%s\t%s\n", procs[i].pid, procs[i].tty, procs[i].stat, procs[i].cmd, procs[i].env);
    }
}

void display_process_tree(my_proc *procs, int count) {
    printf("PID\tTTY\tSTAT\tCMD\n");
    for (int i = 0; i < count; i++) {
        if (procs[i].ppid == 0 || procs[i].ppid == 1) {
            print_tree(procs, count, procs[i].pid, 0);
        }
    }
}

void print_tree(my_proc *procs, int count, int pid, int depth) {
    for (int i = 0; i < count; i++) {
        if (procs[i].ppid == pid) {
            for (int j = 0; j < depth; j++) {
                printf("  ");
            }
            printf("%d\t%s\t%s\t%s\n", procs[i].pid, procs[i].tty, procs[i].stat, procs[i].cmd);
            print_tree(procs, count, procs[i].pid, depth + 1);
        }
    }
}

int cmd_kill(int argc, char **argv) {
    if (argc != 2) return -2;

    pid_t pid = atoi(argv[1]);
    if (pid <= 0) {
        fprintf(stderr, "Invalid PID\n");
        return -1;
    }

    if (kill(pid, SIGKILL) < 0) {
        perror("kill error");
        return -1;
    }

    printf("Process %d killed successfully.\n", pid);
    return 0;
}
