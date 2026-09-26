CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
.PHONY: all clean check
all: spell_line
spell_line: main.c
	$(CC) $(CFLAGS) -o $@ main.c
check: spell_line
	./spell_line
clean:
	rm -f spell_line *.o
