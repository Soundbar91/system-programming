CC = gcc
CFLAGS = -Wall -g -I./header
SRC = source/main.c source/commands.c source/utils.c
OBJ_DIR = build
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
TARGET = final_project-1

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
