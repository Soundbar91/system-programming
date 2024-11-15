CC = gcc
CFLAGS = -Wall -g -I./header

SRC_DIR = source
HEADER_DIR = header

SRC = $(SRC_DIR)/main.c $(SRC_DIR)/commands.c $(SRC_DIR)/util.c
OBJ = $(SRC:.c=.o)

BIN = final_project-1

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(BIN)
