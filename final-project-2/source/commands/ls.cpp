#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <vector>
#include "commands.h"
#include "utils.h"

#define CURRENT_PATH "."

using namespace std;

class file_info {
public:
    struct dirent dir_info;
    struct stat stat_info;

    void print_permissions() {
        printf((S_ISDIR(stat_info.st_mode)) ? "d" : "-");
        printf((stat_info.st_mode & S_IRUSR) ? "r" : "-");
        printf((stat_info.st_mode & S_IWUSR) ? "w" : "-");
        printf((stat_info.st_mode & S_IXUSR) ? "x" : "-");
        printf((stat_info.st_mode & S_IRGRP) ? "r" : "-");
        printf((stat_info.st_mode & S_IWGRP) ? "w" : "-");
        printf((stat_info.st_mode & S_IXGRP) ? "x" : "-");
        printf((stat_info.st_mode & S_IROTH) ? "r" : "-");
        printf((stat_info.st_mode & S_IWOTH) ? "w" : "-");
        printf((stat_info.st_mode & S_IXOTH) ? "x" : "-");
    }
};

int cmd_ls(int argc, char **argv)
{
    if (argc > 2)
    {
        return -2;
    }

    DIR *dp;
    struct dirent *dep;
    vector<file_info> files;

    if ((dp = opendir(CURRENT_PATH)) == NULL)
    {
        perror("opendir error");
        return -1;
    }

    while ((dep = readdir(dp)) != NULL)
    {
        char full_path[MAX_PATH_SIZE];
        file_info file_info;

        memcpy(&file_info.dir_info, dep, sizeof(struct dirent));
        snprintf(full_path, sizeof(full_path), "%s/%s", CURRENT_PATH, dep->d_name);
        stat(full_path, &file_info.stat_info);

        files.push_back(file_info);
    }

    closedir(dp);
    /**
     * TODO. 옵션 설정에 따라 출력 변경
     */
}

void usage_ls(void)
{
}