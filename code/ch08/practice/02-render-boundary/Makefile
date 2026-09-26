CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
.PHONY: all clean check leak
all: render_boundary
render_boundary: game_stub.o render_stub.o
	$(CC) $(CFLAGS) -o $@ game_stub.o render_stub.o
%.o: %.c render_stub.h
	$(CC) $(CFLAGS) -c $< -o $@
# Reading-only exhibit: game calls fake_refresh directly.
# Does NOT model what students should do to the chapter game.
leak: render_boundary_leak
render_boundary_leak: game_leak.c render_stub.c render_stub.h
	$(CC) $(CFLAGS) -DEXPOSE_FAKE_REFRESH -o $@ game_leak.c render_stub.c
check: render_boundary
	./render_boundary
clean:
	rm -f render_boundary render_boundary_leak *.o
