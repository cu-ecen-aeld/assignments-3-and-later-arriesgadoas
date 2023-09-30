CROSS_COMPILER = aarch64-none-linux-gnu-

CC = gcc

CFLAGS = -Wall

SRC = aesdsocket.c

EXEC = aesdsocket

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

clean:
	rm -f $(EXEC)

.PHONY: all clean


