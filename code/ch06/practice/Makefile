CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_struct_init 02_enum_values 03_switch_enum 04_array_of_struct
.PHONY: all clean
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm -f $(DRILLS) *.o
