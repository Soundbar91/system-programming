#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 8085
#define SERVER_ADDR "127.0.0.1"
#define MAX_BUFFER_SIZE 1024

void connect_to_server(int *socket_fd) {
    struct sockaddr_in server_addr;

    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*socket_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);

    if (connect(*socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(*socket_fd);
        exit(EXIT_FAILURE);
    }
}

void execute_command(int socket_fd, const char *command, WINDOW *output_win, char **file_list, int *file_count) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);

    // Send command to server
    if (send(socket_fd, command, strlen(command), 0) < 0) {
        mvwprintw(output_win, 1, 1, "Error sending command: %s", command);
        wrefresh(output_win);
        return;
    }

    // Receive response from server
    ssize_t bytes_received = recv(socket_fd, buffer, MAX_BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        mvwprintw(output_win, 1, 1, "Connection closed by server.");
        wrefresh(output_win);
        return;
    }
    buffer[bytes_received] = '\0';

    // Display server response
    werase(output_win);
    box(output_win, 0, 0);

    // Parse files into file_list
    *file_count = 0;
    char *token = strtok(buffer, "\n");
    while (token != NULL && *file_count < MAX_BUFFER_SIZE) {
        file_list[(*file_count)++] = strdup(token);
        token = strtok(NULL, "\n");
    }

    for (int i = 0; i < *file_count; i++) {
        mvwprintw(output_win, i + 1, 1, "%s", file_list[i]);
    }
    wrefresh(output_win);
}

void free_file_list(char **file_list, int file_count) {
    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }
}

void handle_command_mode(WINDOW *help_win, WINDOW *output_win, int socket_fd, char **file_list, int *file_count, int highlight) {
    char input[256];
    werase(help_win);
    box(help_win, 0, 0);
    mvwprintw(help_win, 1, 1, "Command Mode Active:");
    mvwprintw(help_win, 2, 1, "[r] : Rename File");
    mvwprintw(help_win, 3, 1, "[m] : Create Directory");
    mvwprintw(help_win, 4, 1, "[d] : Delete File");
    mvwprintw(help_win, 5, 1, "[ESC] : Return to Navigation");
    wrefresh(help_win);

    int c = wgetch(help_win);
    switch (c) {
        case 'r': {
            mvwprintw(help_win, 6, 1, "Enter new name for %s: ", file_list[highlight]);
            echo();
            mvwgetnstr(help_win, 7, 1, input, sizeof(input) - 1);
            noecho();
            char command[MAX_BUFFER_SIZE];
            snprintf(command, sizeof(command), "rename %s %s", file_list[highlight], input);
            execute_command(socket_fd, command, output_win, file_list, file_count);
            break;
        }
        case 'm': {
            mvwprintw(help_win, 6, 1, "Enter name of new directory: ");
            echo();
            mvwgetnstr(help_win, 7, 1, input, sizeof(input) - 1);
            noecho();
            char command[MAX_BUFFER_SIZE];
            snprintf(command, sizeof(command), "mkdir %s", input);
            execute_command(socket_fd, command, output_win, file_list, file_count);
            break;
        }
        case 'd': {
            char command[MAX_BUFFER_SIZE];
            snprintf(command, sizeof(command), "rmdir %s", file_list[highlight]);
            execute_command(socket_fd, command, output_win, file_list, file_count);
            break;
        }
        case 27: { // ESC
            return;
        }
        default:
            mvwprintw(help_win, 6, 1, "Invalid command. Press any key to continue.");
            wrefresh(help_win);
            wgetch(help_win);
            break;
    }
}

void navigate_files(WINDOW *output_win, WINDOW *help_win, char **file_list, int file_count, int socket_fd) {
    int highlight = 0;
    int c;

    keypad(output_win, TRUE);
    while (1) {
        werase(output_win);
        box(output_win, 0, 0);
        for (int i = 0; i < file_count; i++) {
            if (i == highlight)
                wattron(output_win, A_REVERSE);
            mvwprintw(output_win, i + 1, 1, "%s", file_list[i]);
            wattroff(output_win, A_REVERSE);
        }
        wrefresh(output_win);

        c = wgetch(output_win);
        switch (c) {
            case KEY_UP:
                highlight = (highlight - 1 + file_count) % file_count;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % file_count;
                break;
            case '\n': {
                char command[MAX_BUFFER_SIZE];
                snprintf(command, sizeof(command), "cd %s", file_list[highlight]);
                if (send(socket_fd, command, strlen(command), 0) < 0) {
                    mvwprintw(output_win, 1, 1, "Error sending command: %s", command);
                    wrefresh(output_win);
                    break;
                }

                // Send ls command after cd
                snprintf(command, sizeof(command), "ls");
                free_file_list(file_list, file_count);
                execute_command(socket_fd, command, output_win, file_list, &file_count);

                if (file_count == 0) {
                    mvwprintw(output_win, 1, 1, "empty");
                    wrefresh(output_win);
                }
                break;
            }
            case KEY_BACKSPACE: {
                char command[MAX_BUFFER_SIZE];
                snprintf(command, sizeof(command), "cd ..");
                if (send(socket_fd, command, strlen(command), 0) < 0) {
                    mvwprintw(output_win, 1, 1, "Error sending command: %s", command);
                    wrefresh(output_win);
                    break;
                }

                // Send ls command after moving up
                snprintf(command, sizeof(command), "ls");
                free_file_list(file_list, file_count);
                execute_command(socket_fd, command, output_win, file_list, &file_count);

                if (file_count == 0) {
                    mvwprintw(output_win, 1, 1, "empty");
                    wrefresh(output_win);
                }
                break;
            }
            case 27: // ESC key
                handle_command_mode(help_win, output_win, socket_fd, file_list, &file_count, highlight);
                execute_command(socket_fd, "ls", output_win, file_list, &file_count);
                break;
        }
    }
}

int main() {
    int socket_fd;
    char *file_list[MAX_BUFFER_SIZE];
    int file_count = 0;

    // Connect to server
    connect_to_server(&socket_fd);

    // Initialize ncurses
    initscr();
    clear();
    noecho();
    cbreak(); // Disable line buffering
    curs_set(0);

    // Get screen dimensions
    int height, width;
    getmaxyx(stdscr, height, width);

    // Create windows for output and help
    WINDOW *output_win = newwin(height - 10, width - 2, 2, 1);
    WINDOW *help_win = newwin(8, width - 2, height - 8, 1);

    keypad(output_win, TRUE);

    // Initial file listing
    execute_command(socket_fd, "ls", output_win, file_list, &file_count);
    navigate_files(output_win, help_win, file_list, file_count, socket_fd);

    free_file_list(file_list, file_count);
    close(socket_fd);
    endwin();
    return 0;
}