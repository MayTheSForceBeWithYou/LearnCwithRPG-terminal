# Chapter 4: Many Files, One Program

## Where we are

`main.c` now holds a title screen, a name prompt, a character sheet, and an
entire coin-flip duel — three unrelated jobs crammed into one file that's
only going to keep growing. Every chapter from here adds more code. If you
don't split the project up now, `main.c` becomes an unreadable wall of text
within a few chapters, and every small change risks breaking something
unrelated three screens away.

## The problem

You need to spread code across multiple files without breaking the program
— which means understanding, precisely, how `gcc` turns *several* `.c`
files into *one* executable, what a header is actually for, and why typing
`gcc *.c -o game` by hand gets old fast. This chapter answers all three, and
you write your first `Makefile` — by hand, line by line, no copy-pasted
magic.

## C concept spotlight

### The compilation model, properly this time

Chapter 1 showed you four stages — preprocess, compile, assemble, link —
using a single file, where linking was invisible: one object file, nothing
to combine it with but the standard library. With multiple files, linking
becomes the interesting part.

Say you split a program into two files, with a shared **header** declaring
the connection between them:

```c
/* greet.h */
#ifndef GREET_H
#define GREET_H

void greet(void);

#endif
```

```c
/* greet.c */
#include <stdio.h>
#include "greet.h"

void greet(void)
{
    printf("hi\n");
}
```

```c
/* main.c */
#include "greet.h"

int main(void)
{
    greet();
    return 0;
}
```

Two things to notice about `#include "greet.h"` versus `#include
<stdio.h>`: quotes mean "look in my own directory first," angle brackets
mean "look in the system's standard library locations." And both `main.c`
and `greet.c` include `greet.h` — `main.c` needs it to know `greet` exists
and what calling it looks like; `greet.c` includes it too, so the compiler
can check that the function it's defining actually matches what was
declared. This is the core idea of a header: it's a **declaration** — "this
function exists, here's its signature" — kept separate from the
**definition** — "here's what it actually does." A `.c` file is called a
**translation unit**: everything the preprocessor assembles for one
`#include`-expanded file, compiled independently of every other `.c` file
in the project.

Compile each `.c` file separately into an object file:

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c main.c -o main.o
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c greet.c -o greet.o
```

Each `.o` is a self-contained chunk of machine code with gaps: `main.o`
contains a call to `greet`, but no idea where `greet`'s actual code lives —
that information doesn't exist yet, because `greet.c` was compiled
completely separately and neither file knows the other exists beyond the
shared header's promise. **Linking** is the step that resolves those gaps:

```bash
gcc main.o greet.o -o prog
./prog
```

```
hi
```

Now watch what happens if you forget to link `greet.o` at all:

```
$ gcc main.o -o prog_bad
/usr/bin/ld: main.o: in function `main':
main.c:5:(.text+0x5): undefined reference to `greet'
collect2: error: ld returned 1 exit status
```

This is the error Chapter 1 promised you'd meet properly. `main.o` compiled
just fine — the header told the compiler enough to trust that `greet`
exists somewhere. But at link time, "somewhere" needs to become an actual
address, and without `greet.o` in the mix, the linker has nowhere to find
one. **Undefined reference to `X`** always means the same thing: something
compiled fine believing `X` exists, and nothing was linked in that actually
defines it. From now on, that message should immediately make you check
your build command (or your `Makefile`) for a missing file, not your
source code for a typo.

### Why every header needs an include guard

```c
#ifndef GREET_H
#define GREET_H

void greet(void);

#endif
```

`#ifndef GREET_H` ("if `GREET_H` is not yet defined") ... `#define GREET_H`
... `#endif` wraps the whole header so its contents only get processed
once per translation unit, no matter how many times it gets `#include`d —
directly, or indirectly through some other header that includes it. Skip
this and you'll eventually include the same header twice in one file
(easy to do by accident once headers start including other headers), and
the compiler will see the same declaration twice. For a plain function
declaration that's harmless — but for a `struct` definition, it's a hard
error:

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -c double_include.c -o /dev/null
In file included from double_include.c:2:
badheader.h:3:3: error: conflicting types for ‘Point’; have ‘struct <anonymous>’
    3 | } Point;
      |   ^~~~~
In file included from double_include.c:1:
badheader.h:3:3: note: previous declaration of ‘Point’ with type ‘Point’
    3 | } Point;
      |   ^~~~~
```

The macro name (`GREET_H` above) just needs to be unique across your whole
project — the usual convention, which this course follows, is the header's
filename, uppercased, with the dot turned into an underscore.

### `static`: keeping a function private to its file

Not every function needs to be usable from other files. `duel.c`, which
you'll write below, has helper functions that only `duel.c` itself needs —
marking them `static` keeps them out of other translation units entirely:

```c
static int flip_coin(void)
{
    return 0;
}
```

A `static` function (at file scope, outside any other function — this is a
different use of `static` from what you'll see elsewhere) has **internal
linkage**: it's invisible outside the `.c` file it's defined in, even if
some other file declares a function with the exact same name. This is
exactly the encapsulation idea from Chapter 3's discussion of scope, one
level up: scope hides a variable inside a function; `static` (used this
way) hides a function inside a file. Only put a function in a header if
other files genuinely need to call it.

## Apply it

Restructure your project into three source files, two of them paired with
headers. If your name/stats code is currently only in `main.c`, this is a
pure reorganization — you're moving code, not rewriting its logic.

**`character.h`** — declares the one thing other files need:

```c
#ifndef CHARACTER_H
#define CHARACTER_H

void character_intro(void);

#endif
```

**`character.c`** — the name prompt and stat sheet, moved here unchanged
from `main.c`, now wrapped in a function with a module-prefixed name:

```c
#include <stdio.h>
#include "character.h"

void character_intro(void)
{
    char name[64];
    printf("What is your name, hero? ");
    fgets(name, sizeof name, stdin);

    int hp = 20;
    int mp = 8;
    int attack = 6;
    int defense = 4;
    char rank = 'E';
    float power_rating = (float)(attack + defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", name);
    printf("----------------------------------------\n");
    printf("HP:     %d\n", hp);
    printf("MP:     %d\n", mp);
    printf("ATK:    %d\n", attack);
    printf("DEF:    %d\n", defense);
    printf("Rank:   %c\n", rank);
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}
```

**`duel.h`**:

```c
#ifndef DUEL_H
#define DUEL_H

void duel_run(void);

#endif
```

**`duel.c`** — same logic as Chapter 3, with `flip_coin` and `ask_call`
now `static` (nothing outside this file needs them) and the public entry
point renamed `duel_run` to match this course's module-prefix convention:

```c
#include <stdio.h>
#include "duel.h"

static int flip_coin(void)
{
    /* Not random yet -- always lands on heads. We fix this for real in
       Chapter 15, once you know how to generate random numbers. */
    return 0;   /* 0 = heads, 1 = tails */
}

static char ask_call(void)
{
    char call = '\0';

    while (call != 'h' && call != 't') {
        printf("Call it: (h)eads or (t)ails? ");

        int typed = getchar();

        switch (typed) {
            case 'h':
            case 'H':
                call = 'h';
                break;
            case 't':
            case 'T':
                call = 't';
                break;
            default:
                break;
        }

        while (typed != '\n' && typed != EOF) {
            typed = getchar();
        }

        if (call != 'h' && call != 't') {
            printf("Didn't catch that -- type h or t.\n");
        }
    }

    return call;
}

void duel_run(void)
{
    printf("\n");
    printf("A stranger flips an old coin and grins.\n");

    char call = ask_call();

    printf("The coin spins");
    for (int i = 0; i < 3; i++) {
        printf(".");
    }
    printf("\n");

    int result = flip_coin();
    char landed = 'h';
    if (result == 1) {
        landed = 't';
    }

    if (landed == 'h') {
        printf("It lands on heads. ");
    } else {
        printf("It lands on tails. ");
    }

    if (call == landed) {
        printf("You called it! You win the duel.\n");
    } else {
        printf("Not what you called. You lose the duel.\n");
    }
}
```

**`main.c`** — shrinks to orchestration:

```c
#include <stdio.h>
#include "character.h"
#include "duel.h"

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*           UNTITLED RPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
    printf("\n");

    character_intro();
    duel_run();

    return 0;
}
```

Now the `Makefile`, explained line by line since you're never getting a
magic one handed to you in this course:

```make
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g

SOURCES = main.c character.c duel.c
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
```

- `CC = gcc` and `CFLAGS = ...` are **variables** — every flag lives in one
  place, so changing your build settings means editing one line, not four.
- `SOURCES = main.c character.c duel.c` lists your `.c` files by hand.
  `OBJECTS = $(SOURCES:.c=.o)` is a **substitution**: it takes `SOURCES`
  and replaces every `.c` suffix with `.o`, giving `main.o character.o
  duel.o` without retyping them.
- `.PHONY: all clean run` tells `make` that `all`, `clean`, and `run` are
  commands to run, not files to check the existence of. Without this, if a
  file named `clean` ever existed in your directory, `make clean` would do
  nothing — `make` would see the file already "exists" and consider its
  job done.
- `all: $(TARGET)` is the **default target** — the one `make` builds when
  you just type `make` with no arguments. A target's job is to say "here's
  what I depend on" (after the colon) and "here's how to build me" (the
  indented recipe lines below it, **which must be indented with an actual
  tab character, not spaces** — this is `make`'s single most notorious
  gotcha, and a mismatched-indentation error is in "Common errors" below).
- `$(TARGET): $(OBJECTS)` says "`game` depends on all three `.o` files;
  rebuild it by linking them." `make` only re-runs this recipe if `game`
  doesn't exist yet, or is older than any of its dependencies.
- `%.o: %.c` is a **pattern rule** — instead of writing separate rules for
  `main.o`, `character.o`, and `duel.o`, this one rule says "any `.o` file
  depends on the `.c` file with the same base name, built this way."
  `$<` and `$@` are automatic variables: `$<` means "the first
  dependency" (the `.c` file), `$@` means "this rule's target" (the
  `.o` file).
- `clean` deletes every generated file, so you can always get back to a
  known state. `run` depends on `$(TARGET)`, so `make run` builds first
  (if needed) and then executes.

Because each object file only rebuilds when its own `.c` file (or
declared dependency) changes, `make` is far faster than a full rebuild once
your project has more than a couple of files — you'll feel this directly
in a moment.

## Compile and run

```bash
make clean
make
```

Expected output — three separate compiles, then one link, in order:

```
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c main.c -o main.o
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c character.c -o character.o
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c duel.c -o duel.o
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.o character.o duel.o
```

```bash
./game
```

Should behave identically to Chapter 3's `game` — same prompts, same
output, just built from four source files instead of one. Now try the
incremental rebuild:

```bash
touch duel.c
make
```

```
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c duel.c -o duel.o
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.o character.o duel.o
```

Only `duel.c` recompiled — `main.o` and `character.o` were untouched, so
`make` trusted them and skipped straight to relinking. And finally:

```bash
make run
```

runs `./game` for you, rebuilding first only if something changed.

## What just happened

Each `.c` file became its own independent island of compiled machine code
(an object file), unaware of the others except through the promises made
in shared headers. Linking is the step that turns three islands into one
program by resolving every function call to an actual address. `make`
exists because remembering "which files changed, so which need
recompiling" by hand doesn't scale — it compares file modification
timestamps (which is exactly why `touch duel.c` was enough to trigger a
rebuild above, with no actual content change needed) and only redoes the
minimum necessary work. One honest limitation worth knowing: this
`Makefile` doesn't track *header* dependencies — if you change `duel.h`
without touching `duel.c` or `main.c`, `make` won't know to rebuild
anything that includes it. `make clean && make` always gets you a correct
build regardless; reach for that whenever you're not sure.

## Common errors

**A tab where a space snuck in (or vice versa) in the `Makefile`:**

```
$ make
Makefile:11: *** missing separator.  Stop.
```

`make` requires recipe lines (the build commands under a target) to start
with a literal tab character — most editors, left to their own devices,
insert spaces instead when you press Tab. If you see `missing separator`,
open the `Makefile` in an editor that shows whitespace, or configure it to
insert real tabs for this one file.

**Undefined reference, revisited — this time from a `Makefile` typo:**

```
$ make
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c main.c -o main.o
gcc -std=c17 -Wall -Wextra -Wpedantic -g -c character.c -o character.o
gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.o character.o
/usr/bin/ld: main.o: in function `main':
main.c:15:(.text+0x5f): undefined reference to `duel_run'
collect2: error: ld returned 1 exit status
make: *** [Makefile:13: game] Error 1
```

(The exact file path, line, and address `ld` reports will reflect your own
directory and file layout — don't expect to match this precisely, just the
shape of it.) Notice `duel.o` is missing from the link command — this is
what happens if `duel.c` gets left out of `SOURCES` in the `Makefile` by
mistake. Compiling succeeds (nothing about `main.c` or `character.c` alone
is wrong), and the error surfaces only at the link step, exactly like the
hand-run example earlier in this chapter. Check your `SOURCES` line first
whenever a `make`-built program reports an undefined reference for a
function you know you wrote.

**Conflicting types from a missing include guard:** see the spotlight
section above — that error is reproduced there verbatim, since it's the
motivating example for why every header needs one.

## Exercises

1. Run `make` twice in a row without changing anything in between. What
   does the second run print? Why?
2. Delete `character.o` (not `character.c`!) and run `make`. Which files
   recompile, and which don't? Does this match your mental model of how
   `make` decides what needs rebuilding?
3. Remove the include guard from `duel.h` entirely (delete all three
   `#ifndef`/`#define`/`#endif` lines) and run `make clean && make`. Does
   it still build? Now add `#include "duel.h"` a second time at the top of
   `duel.c` itself and try again — what breaks, and does the error match
   the "conflicting types" shape from the spotlight, or something else
   (`duel.h` only declares a function, it doesn't define a `struct`)?
4. *Open-ended:* `character.c` and `duel.c` are the only two modules so
   far. Looking at the rest of this course's chapter list, what future
   module do you think will need to know about *both* of them at once? (No
   wrong answers — you're predicting your own project's shape.)

<details>
<summary>Solutions</summary>

1. The second run prints `make: Nothing to be done for 'all'.` (the exact
   wording depends on your `make` version). Every object file and the
   `game` executable are already newer than their dependencies — nothing
   changed, so there's nothing to rebuild, and `make` correctly does no
   work at all.

2. Only `character.o` gets recompiled (its `.c` file exists but its `.o`
   is gone, so the pattern rule fires), followed by relinking `game`
   (since one of its dependencies, `character.o`, is now newer than the
   existing `game` binary). `main.o` and `duel.o` are untouched and
   correctly left alone. This matches `make`'s actual rule exactly:
   compare each target's timestamp against its dependencies', rebuild only
   what's stale.

3. Removing the guard changes nothing by itself — `duel.h` is only
   included once per file today, so there's no double-inclusion happening
   yet for the guard to prevent. Adding a second `#include "duel.h"` at
   the top of `duel.c` reintroduces it artificially. Since `duel.h` only
   *declares* a function (`void duel_run(void);`), and a function
   declaration is allowed to repeat harmlessly, this actually still
   compiles without error — a different outcome from the `struct` example
   in the spotlight. This is worth sitting with: include guards protect
   you from *duplicate struct/typedef definitions* (a hard error) far more
   than from duplicate plain declarations (usually harmless) — but you
   should never rely on that distinction and skip the guard, because a
   header's contents can grow to include a `struct` later without warning.

4. No fixed answer, but a strong candidate: something in the eventual
   `battle.h`/`battle.c` (Chapter 16) will likely need to know about
   character stats, and possibly interact with duel-like win/lose logic.
   You're not expected to know the exact shape yet — this is a prediction
   exercise, not a design commitment.

</details>

## Next up

Your character sheet currently shows one hero's stats with individually
named variables (`hp`, `mp`, `attack`...). A real RPG has a *world* to
walk around in — and a world is naturally a grid. Chapter 5 introduces
arrays, including the two-dimensional kind, and puts a map on screen for
the first time.
