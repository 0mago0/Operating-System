CC=gcc
CFLAGS=-Wall -g
TARGET=unix-shell

all: $(TARGET)

$(TARGET): unix-shell.c
	$(CC) $(CFLAGS) -o $(TARGET) unix-shell.c

clean:
	rm -f $(TARGET)
