CC = gcc

# Where "make install" puts things. PREFIX is the user's choice;
# DESTDIR is prepended to every install path and is how package build
# systems install into a staging directory instead of the live system.
PREFIX  ?= /usr/local
DESTDIR ?=

BINDIR  = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/some-assembly-required

# The installed binary has to know where its data went. Baking the path
# in at compile time is the simplest thing that works, and it is why
# DATADIR must be set before anything is compiled -- not at install
# time, by which point the binary already exists.
# OPT is the only thing the release and sanitizer builds change, so
# DATADIR gets quoted in exactly one place. Nesting quotes through two
# levels of make is a reliable way to lose them.
OPT ?= -g
SAN ?=

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic $(OPT) $(SAN) \
         -DDATADIR='"$(DATADIR)"'
LDLIBS = -lncursesw $(SAN)

SOURCES = main.c paths.c game.c magic.c shop.c save.c entity.c map.c world.c camera.c dialog.c \
          combat_math.c rng.c battle.c party.c inventory.c config.c render_ncurses.c input.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = game

# The test runner links only the pure modules -- no ncurses, no game
# loop. That is a direct benefit of keeping the maths free of the world.
TEST_SOURCES = test_combat.c paths.c combat_math.c rng.c party.c battle.c inventory.c config.c dialog.c magic.c entity.c shop.c save.c world.c map.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_TARGET = run_tests

# The parser fuzzer links the modules that read files, plus the rng
# that generates the garbage to feed them.
FUZZ_SOURCES = fuzz_parsers.c paths.c rng.c map.c config.c save.c party.c magic.c \
               inventory.c entity.c battle.c combat_math.c dialog.c
FUZZ_OBJECTS = $(FUZZ_SOURCES:.c=.o)
FUZZ_TARGET = fuzz_parsers

.PHONY: all clean run test valgrind sanitize fuzz fuzz-sanitize \
        install uninstall dist release

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
	$(MAKE) SAN=-fsanitize=address,undefined

# Thousands of random files through every parser. Run it under the
# sanitizers -- a parser bug is an out-of-bounds read, which is
# exactly what ASan is best at.
fuzz: $(FUZZ_TARGET)
	./$(FUZZ_TARGET)

fuzz-sanitize:
	$(MAKE) clean
	$(MAKE) $(FUZZ_TARGET) SAN=-fsanitize=address,undefined
	./$(FUZZ_TARGET)

# A build for other people: optimised, and without the -g debug info
# that triples the binary size and helps nobody but you.
release:
	$(MAKE) clean
	$(MAKE) OPT='-O2 -DNDEBUG'

# install -D creates the parent directories; -m sets the mode
# explicitly rather than inheriting whatever the umask happens to be.
install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -Dm644 assets/levels.txt $(DESTDIR)$(DATADIR)/levels.txt
	install -Dm644 assets/spells.txt $(DESTDIR)$(DATADIR)/spells.txt
	install -Dm644 assets/config.txt $(DESTDIR)$(DATADIR)/config.txt
	for m in assets/maps/*.map; do \
	    install -Dm644 "$$m" $(DESTDIR)$(DATADIR)/maps/$$(basename $$m); \
	done
	@echo "Installed to $(DESTDIR)$(BINDIR)/$(TARGET)"
	@echo "Data in      $(DESTDIR)$(DATADIR)"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -rf $(DESTDIR)$(DATADIR)
	@echo "Removed. Saves in ~/.local/share are left alone."

# A source tarball that unpacks into its own directory, which is the
# one convention every "tar xf" user relies on.
DISTNAME = some-assembly-required-1.0
dist: clean
	mkdir -p $(DISTNAME)
	cp -r *.c *.h Makefile valgrind.supp assets $(DISTNAME)/
	tar czf $(DISTNAME).tar.gz $(DISTNAME)
	rm -rf $(DISTNAME)
	@echo "Created $(DISTNAME).tar.gz"

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(FUZZ_OBJECTS) \
	      $(TARGET) $(TEST_TARGET) $(FUZZ_TARGET)
	rm -rf $(DISTNAME) $(DISTNAME).tar.gz fuzz_tmp.txt

run: $(TARGET)
	./$(TARGET)
