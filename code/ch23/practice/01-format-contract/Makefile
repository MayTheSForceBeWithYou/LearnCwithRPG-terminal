CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
TARGET = format_contract
.PHONY: all clean run bad check
all: $(TARGET)
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c
check: $(TARGET)
	./$(TARGET)
bad:
	@echo Attempting compile WITHOUT format attribute...
	-$(CC) $(CFLAGS) -Werror=format -DNO_FORMAT_ATTR -o bad_call bad_call.c 2>bad_without.txt; echo without_exit:$$?
	@echo Attempting compile WITH format attribute...
	-$(CC) $(CFLAGS) -Werror=format -o bad_call_attr bad_call.c 2>bad_with.txt; echo with_exit:$$?
	@echo --- without attr ---; cat bad_without.txt || true
	@echo --- with attr ---; cat bad_with.txt || true
clean:
	rm -f $(TARGET) bad_call bad_call_attr bad_without.txt bad_with.txt *.o
run: all
	./$(TARGET)
