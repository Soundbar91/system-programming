#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 8085
#define SERVER_ADDR "127.0.0.1"
#define MAX_BUFFER_SIZE 1024

char *clipboard_file = NULL; // For storing the file to copy
char current_path[MAX_BUFFER_SIZE] = "/tmp/test"; // Track the current directory

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

void execute_ls_command(int socket_fd, const char *command, WINDOW *output_win, char **file_list, int *file_count) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);

    // Send ls -l command to server
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

    // Clear and refresh output window after receiving response
    werase(output_win);
    box(output_win, 0, 0);

    // Parse files into file_list
    *file_count = 0;
    char *token = strtok(buffer, "\n");
    while (token != NULL && *file_count < MAX_BUFFER_SIZE) {
        // Save the full line of ls -l output to file_list
        file_list[(*file_count)++] = strdup(token);

        // Display the full ls -l output in the window
        mvwprintw(output_win, *file_count, 1, "%s", token);
        token = strtok(NULL, "\n");
    }

    if (*file_count == 0) {
        mvwprintw(output_win, 1, 1, "empty");
    }
    wrefresh(output_win);
}

char *extract_file_name(const char *ls_line) {
    // Find the last space in the line to extract the file/directory name
    char *last_space = strrchr(ls_line, ' ');
    if (last_space) {
        return strdup(last_space + 1); // Return a copy of the extracted name
    }
    return NULL;
}

void free_file_list(char **file_list, int file_count) {
    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }
}

void refresh_file_list(int socket_fd, WINDOW *output_win, char **file_list, int *file_count) {
    free_file_list(file_list, *file_count);

    char command[MAX_BUFFER_SIZE];
    snprintf(command, sizeof(command), "ls -l %s", current_path);
    execute_ls_command(socket_fd, command, output_win, file_list, file_count);
}

void execute_non_ls_command(int socket_fd, const char *command) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);

    // Send command to server
    if (send(socket_fd, command, strlen(command), 0) < 0) {
        perror("send");
        return;
    }

    // Receive response from server (optional, not displayed)
    recv(socket_fd, buffer, MAX_BUFFER_SIZE - 1, 0);
}

void handle_command_mode(WINDOW *help_win, WINDOW *output_win, int socket_fd, char **file_list, int *file_count, int highlight) {
    char input[256];
    werase(help_win);
    box(help_win, 0, 0);
    mvwprintw(help_win, 1, 1, "Command Mode Active:");
    mvwprintw(help_win, 2, 1, "[r] : Rename   [m] : Create Dir   [d] : Delete");
    mvwprintw(help_win, 3, 1, "[c] : Chmod    [p] : Copy         [v] : Paste");
    mvwprintw(help_win, 4, 1, "[M] : Move     [ESC] : Back to Navigation");
    wrefresh(help_win);

    int c = wgetch(help_win);
    switch (c) {
        case 'r': {
            char *file_name = extract_file_name(file_list[highlight]);
            if (file_name) {
                mvwprintw(help_win, 9, 1, "Enter new name for %s: ", file_name);
                echo();
                curs_set(1);
                mvwgetnstr(help_win, 10, 1, input, sizeof(input) - 1);
                noecho();
                curs_set(0);
                char command[MAX_BUFFER_SIZE];
                snprintf(command, sizeof(command), "rename %s/%s %s/%s", current_path, file_name, current_path, input);
                free(file_name);
                execute_non_ls_command(socket_fd, command);
                refresh_file_list(socket_fd, output_win, file_list, file_count);
            }
            break;
        }
        case 'm': {
            mvwprintw(help_win, 9, 1, "Enter name of new directory: ");
            echo();
            curs_set(1);
            mvwgetnstr(help_win, 10, 1, input, sizeof(input) - 1);
            noecho();
            curs_set(0);
            char command[MAX_BUFFER_SIZE];
            snprintf(command, sizeof(command), "mkdir %s/%s", current_path, input);
            execute_non_ls_command(socket_fd, command);
            refresh_file_list(socket_fd, output_win, file_list, file_count);
            break;
        }
        case 'd': {
            char *file_name = extract_file_name(file_list[highlight]);
            if (file_name) {
                char command[MAX_BUFFER_SIZE];
                snprintf(command, sizeof(command), "rmdir %s/%s", current_path, file_name);
                free(file_name);
                execute_non_ls_command(socket_fd, command);
                refresh_file_list(socket_fd, output_win, file_list, file_count);
            }
            break;
        }
        case 'c': {
            char *file_name = extract_file_name(file_list[highlight]);
            if (file_name) {
                mvwprintw(help_win, 9, 1, "Enter permissions for %s (e.g., 777): ", file_name);
                echo();
                curs_set(1);
                mvwgetnstr(help_win, 10, 1, input, sizeof(input) - 1);
                noecho();
                curs_set(0);
                char command[MAX_BUFFER_SIZE];
                snprintf(command, sizeof(command), "chmod %s %s/%s", input, current_path, file_name);
                free(file_name);
                execute_non_ls_command(socket_fd, command);
                refresh_file_list(socket_fd, output_win, file_list, file_count);
            }
            break;
        }
        case 'p': {
            char *file_name = extract_file_name(file_list[highlight]);
            if (file_name) {
                clipboard_file = malloc(MAX_BUFFER_SIZE);
                snprintf(clipboard_file, MAX_BUFFER_SIZE, "%s/%s", current_path, file_name); // Save the file name to clipboard
                mvwprintw(help_win, 9, 1, "File '%s' ready to copy. Navigate to target directory and press 'v' to paste.", file_name);
                wrefresh(help_win);
                free(file_name);
            }
            break;
        }
        case 'v': {
            if (clipboard_file) {
                mvwprintw(help_win, 9, 1, "Pasting '%s' to current directory...", clipboard_file);
                wrefresh(help_win);

                // Create copy command
                char command[MAX_BUFFER_SIZE];
                char *file_name = extract_file_name(file_list[highlight]);
                snprintf(command, sizeof(command), "cp %s %s/%s", clipboard_file, current_path, file_name);
                execute_non_ls_command(socket_fd, command);
                refresh_file_list(socket_fd, output_win, file_list, file_count);

                free(clipboard_file);
                clipboard_file = NULL; // Clear clipboard
            } else {
                mvwprintw(help_win, 9, 1, "No file to paste. Use 'p' to select a file first.");
                wrefresh(help_win);
            }
            break;
        }
        case 'M': {
            if (clipboard_file) {
                mvwprintw(help_win, 9, 1, "Moving '%s' to current directory...", clipboard_file);
                wrefresh(help_win);

                // Create move command
                char command[MAX_BUFFER_SIZE];
                snprintf(command, sizeof(command), "mv %s %s", clipboard_file, current_path);
                execute_non_ls_command(socket_fd, command);
                refresh_file_list(socket_fd, output_win, file_list, file_count);

                free(clipboard_file);
                clipboard_file = NULL; // Clear clipboard
            } else {
                mvwprintw(help_win, 9, 1, "No file to move. Use 'p' to select a file first.");
                wrefresh(help_win);
            }
            break;
        }
        case 27: { // ESC
            break;
        }
        default:
            mvwprintw(help_win, 9, 1, "Invalid command. Press any key to continue.");
            wrefresh(help_win);
            wgetch(help_win);
            break;
    }

    // Clear help window after command mode ends
    werase(help_win);
    box(help_win, 0, 0);
    wrefresh(help_win);
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
                char *file_name = extract_file_name(file_list[highlight]);
                if (file_name) {
                    char command[MAX_BUFFER_SIZE];
                    snprintf(command, sizeof(command), "cd %s/%s", current_path, file_name);
                    if (strlen(current_path) + strlen(file_name) + 2 < MAX_BUFFER_SIZE) {
                        strcat(current_path, "/");
                        strcat(current_path, file_name); // Update current path
                    }
                    free(file_name);
                    execute_non_ls_command(socket_fd, command);
                    refresh_file_list(socket_fd, output_win, file_list, &file_count);
                }
                break;
            }
            case KEY_BACKSPACE: {
                if (strcmp(current_path, "/tmp/test") != 0) { // Prevent going above root directory
                    char *last_slash = strrchr(current_path, '/');
                    if (last_slash) {
                        *last_slash = '\0'; // Update current path to parent directory
                    }
                }
                char command[MAX_BUFFER_SIZE];
                snprintf(command, sizeof(command), "cd %s", current_path);
                execute_non_ls_command(socket_fd, command);
                refresh_file_list(socket_fd, output_win, file_list, &file_count);
                break;
            }
            case 27: // ESC key
                handle_command_mode(help_win, output_win, socket_fd, file_list, &file_count, highlight);
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

    // Adjust dimensions
    int height, width;
    getmaxyx(stdscr, height, width);
    int output_height = height - 12;
    int help_height = 10;

    // Create windows for output and help
    WINDOW *output_win = newwin(output_height, width - 2, 1, 1);
    WINDOW *help_win = newwin(help_height, width - 2, height - help_height - 1, 1);

    keypad(output_win, TRUE);

    // Initial file listing
    execute_ls_command(socket_fd, "ls -l /tmp/test", output_win, file_list, &file_count);
    navigate_files(output_win, help_win, file_list, file_count, socket_fd);

    free_file_list(file_list, file_count);
    close(socket_fd);
    endwin();
    return 0;
}
