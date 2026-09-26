CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_assert_floor 02_ndebug_toy 03_fuzz_tokens
.PHONY: all clean check
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
check: all
	./01_assert_floor
	./02_ndebug_toy
	./03_fuzz_tokens
clean:
	rm -f $(DRILLS) *.o
