CC = clang
CFLAGS = -Wall -Wextra -Werror -Iinclude
LDFLAGS =

SRC = src/main.c src/board.c src/simulator.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = snakes_and_ladders

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
