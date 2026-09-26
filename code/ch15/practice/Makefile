CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_clamp_damage 02_flee_chance 03_monotonic 04_xorshift_seed
.PHONY: all clean check
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
check: all
	./01_clamp_damage
	./02_flee_chance
	./03_monotonic
	./04_xorshift_seed
clean:
	rm -f $(DRILLS) *.o
