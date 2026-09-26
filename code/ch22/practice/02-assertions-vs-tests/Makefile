CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
TARGET = assertions_vs_tests
.PHONY: all clean run demo-assert check
all: $(TARGET)
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c
demo-assert: main.c
	$(CC) $(CFLAGS) -DDEMO_BAD_FLOOR -o $(TARGET)_bad main.c
check: $(TARGET)
	./$(TARGET)
clean:
	rm -f $(TARGET) $(TARGET)_bad *.o
run: all
	./$(TARGET)
