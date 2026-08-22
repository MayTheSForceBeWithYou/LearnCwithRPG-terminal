CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g

SOURCES = main.c entity.c map.c duel.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = game

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
