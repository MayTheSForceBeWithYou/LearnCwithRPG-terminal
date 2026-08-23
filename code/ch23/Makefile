CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
LDLIBS = -lncursesw

SOURCES = main.c game.c magic.c shop.c save.c entity.c map.c world.c camera.c dialog.c \
          combat_math.c rng.c battle.c party.c inventory.c config.c render_ncurses.c input.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = game

# The test runner links only the pure modules -- no ncurses, no game
# loop. That is a direct benefit of keeping the maths free of the world.
TEST_SOURCES = test_combat.c combat_math.c rng.c party.c battle.c inventory.c config.c dialog.c magic.c entity.c shop.c save.c world.c map.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_TARGET = run_tests

# The parser fuzzer links the modules that read files, plus the rng
# that generates the garbage to feed them.
FUZZ_SOURCES = fuzz_parsers.c rng.c map.c config.c save.c party.c magic.c \
               inventory.c entity.c battle.c combat_math.c dialog.c
FUZZ_OBJECTS = $(FUZZ_SOURCES:.c=.o)
FUZZ_TARGET = fuzz_parsers

.PHONY: all clean run test valgrind sanitize fuzz fuzz-sanitize

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDLIBS)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_OBJECTS)

$(FUZZ_TARGET): $(FUZZ_OBJECTS)
	$(CC) $(CFLAGS) -o $(FUZZ_TARGET) $(FUZZ_OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

# The test suite under memcheck. This is the one that catches
# uninitialised reads, which the sanitizers do not.
valgrind: $(TEST_TARGET)
	valgrind --leak-check=full --track-origins=yes \
	         --suppressions=valgrind.supp --error-exitcode=9 ./$(TEST_TARGET)

# The whole game under the sanitizers, which catch what memcheck is slow
# at: out-of-bounds and use-after-free.
sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
	        LDLIBS="-lncursesw -fsanitize=address,undefined"

# Thousands of random files through every parser. Run it under the
# sanitizers -- a parser bug is an out-of-bounds read, which is
# exactly what ASan is best at.
fuzz: $(FUZZ_TARGET)
	./$(FUZZ_TARGET)

fuzz-sanitize:
	$(MAKE) clean
	$(MAKE) $(FUZZ_TARGET) \
	        CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined"
	./$(FUZZ_TARGET)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(FUZZ_OBJECTS) \
	      $(TARGET) $(TEST_TARGET) $(FUZZ_TARGET)

run: $(TARGET)
	./$(TARGET)
