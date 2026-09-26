CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
.PHONY: all clean check
all: glyph_table
glyph_table: main.c
	$(CC) $(CFLAGS) -o $@ main.c
check: glyph_table
	./glyph_table
clean:
	rm -f glyph_table *.o
