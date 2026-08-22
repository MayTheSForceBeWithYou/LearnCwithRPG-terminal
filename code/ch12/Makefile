CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
LDLIBS = -lncursesw

SOURCES = main.c entity.c map.c camera.c dialog.c duel.c render_ncurses.c input.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = game

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
