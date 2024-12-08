#include "commands.h"
#include "utils.h"

#define CURRENT_PATH "."
#define MAX_FILES 1024 

typedef struct file_info {
    struct dirent dir_info;
    struct stat stat_info;
} file_info;

void print_permissions(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

int cmd_ls(int argc, char **argv) {
    if (argc > 2)
    {
        return -2;
    }

    DIR *dp;
    struct dirent *dep;
    file_info files[MAX_FILES];
    int file_count = 0;

    if ((dp = opendir(CURRENT_PATH)) == NULL) {
        perror("opendir error");
        return -1;
    }

    while ((dep = readdir(dp)) != NULL) {
        if (file_count >= MAX_FILES) {
            fprintf(stderr, "Too many files. Increase MAX_FILES.\n");
            break;
        }

        char full_path[MAX_PATH_SIZE];
        file_info *current_file = &files[file_count++];

        memcpy(&current_file->dir_info, dep, sizeof(struct dirent));

        snprintf(full_path, sizeof(full_path), "%s/%s", CURRENT_PATH, dep->d_name);

        if (stat(full_path, &current_file->stat_info) < 0) {
            perror("stat error");
            file_count--;
        }
    }

    closedir(dp);

    /**
     * TODO. 옵션 설정에 따라 출력 변경
     */

    return 0;
}

void usage_ls(void) {
    printf("ls [options]\n");
    printf("Display the contents of the current directory.\n");
    printf("Options:\n");
    printf("  -l   Display detailed information (not implemented yet).\n");
    printf("Examples:\n");
    printf("  ls\n");
}
