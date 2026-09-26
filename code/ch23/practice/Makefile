CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_format_attr 02_phase_index 03_sim_potion_gate
.PHONY: all clean check
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
check: all
	./01_format_attr
	./02_phase_index
	./03_sim_potion_gate
clean:
	rm -f $(DRILLS) *.o
