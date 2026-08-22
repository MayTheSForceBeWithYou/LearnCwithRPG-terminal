CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
LDLIBS = -lncursesw

SOURCES = main.c game.c magic.c entity.c map.c world.c camera.c dialog.c \
          combat_math.c rng.c battle.c party.c inventory.c config.c duel.c render_ncurses.c input.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = game

# The test runner links only the pure modules -- no ncurses, no game
# loop. That is a direct benefit of keeping the maths free of the world.
TEST_SOURCES = test_combat.c combat_math.c rng.c party.c battle.c inventory.c config.c dialog.c magic.c entity.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_TARGET = run_tests

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDLIBS)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)
