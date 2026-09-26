CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = dead_zone

.PHONY: all clean run check

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

check: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
