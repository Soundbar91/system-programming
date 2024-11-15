#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_SIZE    (128)

int main(int argc, char **argv)
{
    char *command, *tok_str;
    char *current_dir = "/";

    command = (char*) malloc(MAX_CMD_SIZE);
    if (command == NULL) {
        perror("malloc");
        exit(1);
    }

    do {
        printf("%s $ ", current_dir);
        if (fgets(command, MAX_CMD_SIZE-1, stdin) == NULL) break;

        tok_str = strtok(command, " \n");
        if (tok_str == NULL) continue;

        if (strcmp(tok_str, "quit") == 0) {
            break;
        } else {
            // TODO: implement functions
            printf("your command: %s\n", tok_str);
            printf("and argument is ");

            tok_str = strtok(NULL, " \n");
            if (tok_str == NULL) {
                printf("NULL\n");
            } else {
                printf("%s\n", tok_str);
            }
        }
    } while (1);

    free(command);

    return 0;
}
