CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_switch_call 02_while_countdown 03_for_sum 04_validate_loop
.PHONY: all clean
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm -f $(DRILLS) *.o
