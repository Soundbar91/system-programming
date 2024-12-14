#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "shell.h"
#include "utils.h"
#include "commands.h"

#define SERVER_PORT 8085

// Path information
char *chroot_path = "/tmp/test";
char *current_dir;

/* 커스텀 쉘 모드
int main(int argc, char **argv)
{
    char command[MAX_CMDLINE_SIZE];
    char *cmd_argv[MAX_ARG];
    int cmd_argc;

    setup_signal_handling();

    current_dir = malloc(MAX_CMDLINE_SIZE);
    if (!current_dir)
    {
        perror("malloc");
        return 1;
    }

    if (chdir(chroot_path) < 0)
    {
        mkdir(chroot_path, 0755);
        chdir(chroot_path);
    }

    while (1)
    {
        if (getcwd(current_dir, MAX_CMDLINE_SIZE) == NULL)
        {
            perror("getcwd");
            break;
        }

        printf("%s$ ", current_dir + strlen(chroot_path));
        if (!fgets(command, MAX_CMDLINE_SIZE, stdin))
            break;

        char *tok = strtok(command, " \n");
        if (!tok) {
            continue;
        }

        cmd_argv[0] = tok;
        for (cmd_argc = 1; cmd_argc < MAX_ARG; cmd_argc++)
        {
            if (!(tok = strtok(NULL, " \n"))) {
                break;
            }
            cmd_argv[cmd_argc] = tok;
        }

        int cmd_index = search_command(cmd_argv[0]);
        if (cmd_index < 0)
        {
            printf("%s: command not found\n", cmd_argv[0]);
        }
        else
        {
            int result = cmd_list[cmd_index].cmd_func(cmd_argc, cmd_argv);
            if (result == -2) {
                perror("argument valid");
            }
        }
    }

    free(current_dir);
    return 0;
}*/

void setup_server_socket(int *server_fd, int *client_fd)
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // 서버 소켓 생성
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 소켓 바인딩
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    // 소켓 리스닝
    if (listen(*server_fd, 5) < 0)
    {
        perror("listen");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", SERVER_PORT);

    // 클라이언트 연결 수락
    *client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0)
    {
        perror("accept");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Client connected.\n");
}

int main()
{
    char command[MAX_CMDLINE_SIZE];
    char *cmd_argv[MAX_ARG];
    int cmd_argc;
    int server_fd, client_fd;
    char buffer[MAX_BUFFER_SIZE];

    FILE *redirect_file = tmpfile();
    if (!redirect_file)
    {
        perror("tmpfile");
        exit(EXIT_FAILURE);
    }

    int redirect_fd = fileno(redirect_file);

    int original_stdout_fd = dup(fileno(stdout));
    fflush(stdout);
    dup2(redirect_fd, fileno(stdout));

    setup_signal_handling();
    current_dir = malloc(MAX_CMDLINE_SIZE);
    if (!current_dir)
    {
        perror("malloc");
        return 1;
    }

    if (chdir(chroot_path) < 0)
    {
        mkdir(chroot_path, 0755);
        chdir(chroot_path);
    }

    setup_server_socket(&server_fd, &client_fd);

    while (1)
    {
        if (getcwd(current_dir, MAX_CMDLINE_SIZE) == NULL)
        {
            perror("getcwd");
            break;
        }

        memset(buffer, 0, MAX_BUFFER_SIZE);
        ssize_t bytes_received = recv(client_fd, buffer, MAX_BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Received command: %s\n", buffer);

        strncpy(command, buffer, MAX_CMDLINE_SIZE);

        char *tok = strtok(command, " \n");
        if (!tok)
        {
            snprintf(buffer, MAX_BUFFER_SIZE, "Invalid command\n");
            send(client_fd, buffer, strlen(buffer), 0);
            continue;
        }

        cmd_argv[0] = tok;
        for (cmd_argc = 1; cmd_argc < MAX_ARG; cmd_argc++)
        {
            if (!(tok = strtok(NULL, " \n")))
            {
                break;
            }
            cmd_argv[cmd_argc] = tok;
        }

        int cmd_index = search_command(cmd_argv[0]);
        if (cmd_index < 0)
        {
            snprintf(buffer, MAX_BUFFER_SIZE, "%s: command not found\n", cmd_argv[0]);
        }
        else
        {
            fflush(stdout);
            ftruncate(redirect_fd, 0);
            rewind(redirect_file);

            cmd_list[cmd_index].cmd_func(cmd_argc, cmd_argv);

            fflush(stdout);
            rewind(redirect_file);

            size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE - 1, redirect_file);
            if (bytes_read == 0)
            {
                snprintf(buffer, MAX_BUFFER_SIZE, "is Empty\n");
            }
            else
            {
                buffer[bytes_read] = '\0'; // 읽은 데이터에 NULL 종료 문자 추가
            }
        }

        send(client_fd, buffer, strlen(buffer), 0);
    }

    fflush(stdout);
    dup2(original_stdout_fd, fileno(stdout));
    close(original_stdout_fd);

    fclose(redirect_file);
    close(client_fd);
    close(server_fd);
    free(current_dir);
    return 0;
}
