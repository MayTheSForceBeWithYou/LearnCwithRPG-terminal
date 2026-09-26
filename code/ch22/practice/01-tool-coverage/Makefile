CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
TARGET = tool_coverage
.PHONY: all clean run check
all: $(TARGET)
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c
check: $(TARGET)
	./$(TARGET)
clean:
	rm -f $(TARGET) *.o
run: all
	./$(TARGET)
