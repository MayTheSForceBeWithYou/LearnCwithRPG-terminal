CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
DRILLS = 01_buy_commit 02_sell_commit 03_discount_price 04_rest_no_save
.PHONY: all clean
all: $(DRILLS)
%: %.c
	$(CC) $(CFLAGS) -o $@ $<
clean:
	rm -f $(DRILLS) *.o
