CC = gcc
CFLAGS = -Wall -g -I./header
SRC = source/main.c source/commands.c source/utils.c
OBJ = $(SRC:.c=.o)
TARGET = final_project-1

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
