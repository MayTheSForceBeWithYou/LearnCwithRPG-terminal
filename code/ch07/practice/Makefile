CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_swap 02_minmax 03_walk_array 04_const_contract 05_dangling_demo
.PHONY: all clean
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm -f $(DRILLS) *.o
