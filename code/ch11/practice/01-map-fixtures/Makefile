CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_header 02_row_width 03_skip_comments
.PHONY: all clean check
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
check: all
	./01_header
	./02_row_width
	./03_skip_comments
clean:
	rm -f $(DRILLS) *.o
