CC = gcc
CFLAGS = -Wall -g -I./header

SRC_DIR = source
CMD_DIR = $(SRC_DIR)/commands
OBJ_DIR = build

MAIN_SRC = main.c
SRCS = $(MAIN_SRC) $(wildcard $(SRC_DIR)/*.c $(CMD_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = final_project-2

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(CMD_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
