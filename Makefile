CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude -pthread
TARGET := minios

SRC := \
	src/main.c \
	src/minios.c \
	src/fs.c \
	src/shell.c \
	src/storage.c \
	src/threading.c \
	src/commands/ls.c \
	src/commands/cd.c \
	src/commands/mkdir.c \
	src/commands/cat.c \
	src/commands/pwd.c \
	src/commands/chmod.c \
	src/commands/clear.c \
	src/commands/cp.c \
	src/commands/rmdir.c \
	src/commands/echo.c \
	src/commands/search.c

OBJ := $(SRC:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
