CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
.PHONY: all clean check
all: key_dispatch
key_dispatch: main.c
	$(CC) $(CFLAGS) -o $@ main.c
check: key_dispatch
	./key_dispatch
clean:
	rm -f key_dispatch *.o
