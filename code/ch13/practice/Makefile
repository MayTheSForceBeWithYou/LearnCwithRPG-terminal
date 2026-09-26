CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_enum_index 02_linear_search 03_validate_coords 04_count_slots
.PHONY: all clean
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm -f $(DRILLS) *.o
