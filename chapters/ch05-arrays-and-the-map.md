# Chapter 5: Arrays and the Map

## Where we are

Your project is now three modules — `character`, `duel`, and the `main`
that orchestrates them — built by a `Makefile` that only rebuilds what
changed. The game asks for a name, shows a stat sheet, and runs a duel. But
there's still no *world*: nowhere to stand, nothing to walk across. This
chapter introduces arrays — including the two-dimensional kind — and uses
one to put an actual map on screen for the first time.

## The problem

Chapter 2 gave you a fixed-size `char name[64]` buffer without explaining
how it really works, promising a real answer later. Later is now. An RPG's
world is naturally a grid — rows and columns of tiles — and C's tool for
"many values of the same type, addressed by position" is the array. You
need to understand indexing precisely, including its sharpest edge: C does
not stop you from indexing past the end of one, and the consequences are
worse than an error message.

## C concept spotlight

### One-dimensional arrays

```c
int scores[3] = {10, 20, 30};

printf("%d\n", scores[0]);   /* 10 */
printf("%d\n", scores[1]);   /* 20 */
printf("%d\n", scores[2]);   /* 30 */
```

`int scores[3]` reserves space for exactly three `int`s, laid out
back-to-back in memory — no gaps, no reordering. `scores[0]` is the first
one. **Indexing starts at 0, not 1** — this trips up nearly everyone
coming from everyday counting, and there's a real reason for it, not just
tradition: `scores[i]` really means "start at `scores`'s address and move
forward `i` slots." The first slot is 0 slots forward from the start.
You'll feel the full weight of that explanation once Chapter 7 covers
pointers directly; for now, just build the habit that a 3-element array's
valid indices are `0`, `1`, `2` — never `3`.

`char name[64]` from Chapter 2 was exactly this: an array of 64 `char`s. No
new concept — just a bigger, differently-typed version of `scores` above,
which is why we could use it back then without a full explanation.

### Two-dimensional arrays

A grid is just an array of arrays:

```c
#define MAP_HEIGHT 5
#define MAP_WIDTH  10

char world[MAP_HEIGHT][MAP_WIDTH] = {
    { '#','#','#','#','#','#','#','#','#','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','.','.','#','#','#','#','.','.','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','#','#','#','#','#','#','#','#','#' }
};
```

`world[row][col]` reads as "the `col`-th character of the `row`-th row."
Notice each row is written as its own `{ ... }` list of individual
characters, not a string in double quotes — that's deliberate. A string
literal like `"#........#"` carries an implicit terminating byte (you'll
learn exactly what that byte is and why it matters in Chapter 12), and
current `gcc` actually warns if you try to cram an 11-character string
(10 visible characters plus that hidden terminator) into a 10-wide row:

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -c badmap.c -o /dev/null
badmap.c:9:9: warning: initializer-string for array of ‘char’ truncates NUL terminator but destination lacks ‘nonstring’ attribute (11 chars into 10 available) [-Wunterminated-string-initialization]
    9 |         "##########",
      |         ^~~~~~~~~~~~
```

Since this map isn't a string yet — just a grid of individual characters —
initializing it character-by-character sidesteps that warning entirely and
is more honest about what's actually being stored.

Printing the grid needs two nested loops — one for rows, one for columns
inside each row:

```c
for (int row = 0; row < MAP_HEIGHT; row++) {
    for (int col = 0; col < MAP_WIDTH; col++) {
        printf("%c", world[row][col]);
    }
    printf("\n");
}
```

The inner loop walks one row left to right, printing each character with
no newline; the outer loop's `printf("\n")` runs once per row, after the
inner loop finishes it, moving to the next line on screen.

### Out of bounds: the array's sharp edge

Here's the part every other language you've used would have stopped you
from doing, and C simply doesn't:

```c
int scores[3] = {10, 20, 30};

printf("scores[5] = %d\n", scores[5]);
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o oob oob.c
$ ./oob
scores[5] = -493898576
```

Zero warnings, zero errors, a clean compile — and a number that was never
put there on purpose. `scores[5]` computed a memory address five slots past
where `scores` starts and read whatever bit pattern happened to already be
sitting there, left over from something else entirely. This is **undefined
behaviour**: the C standard places no requirement whatsoever on what
happens when you index outside an array's bounds. It might print garbage
(as here). It might print a value that happens to look reasonable, hiding
the bug from you. Or — if you *write* out of bounds instead of just
reading — it can be far more dramatic:

```c
int guard = 42;
int scores[3] = {10, 20, 30};

scores[3] = 999;   /* one past the end -- still compiles clean */

printf("guard = %d\n", guard);
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o oob2 oob2.c
$ ./oob2
*** stack smashing detected ***: terminated
```

This particular crash is your compiler's **stack protector** — a runtime
safety net, on by default on this toolchain, that plants a hidden guard
value near local arrays specifically to catch exactly this kind of
overwrite and abort before anything worse happens. It's a genuinely good
thing that this crashed loudly. But don't read that as "C catches this for
you" — the stack protector doesn't catch every out-of-bounds write (it
depends on exactly what memory sits where, which varies by compiler,
platform, and even which variables happen to be adjacent), and it does
nothing at all for out-of-bounds *reads* like the first example. The only
real guarantee is the one already stated: indexing outside an array's
bounds is undefined behaviour, full stop, and the only fix is to never let
it happen — by construction, with `for` loops whose bounds you've checked,
not by hoping a safety net catches your mistake.


### Flat layout and the offset formula

`world[row][col]` looks like two lookups. Internally it is one:

```
offset = row * MAP_WIDTH + col
```

A 5×10 map is fifty contiguous `char`s. Row 2, column 3 is byte
`2 * 10 + 3 = 23` from the start — nowhere is there a separate "row object"
with its own bounds check. That is why an off-by-one on the *row* loop is
the same family of bug as `scores[5]`: the arithmetic happily walks past
the block.

**Loop shape that matches the layout:** outer loop on `row` (or `y`),
inner on `col` (or `x`). Printing left-to-right, top-to-bottom matches
how humans read a map *and* how the bytes are packed. Swapping the loops
still "works" for a square, but the mental model and any later
`offset = y * width + x` helpers will fight you.

**Defensive indexing.** When a coordinate comes from outside the module
(player input, a camera, a file), clamp or reject *before* subscripting:

```c
if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
    return '#';   /* or 0 / error — pick a policy and document it */
}
return world[y][x];
```

Chapter 9's camera clamp and Chapter 7's `map_is_walkable` both rest on
this habit. The drills in `code/ch05/practice/` force the offset math and
a tiny `clamp_index` until they are automatic — do those before you treat
map edits as the only practice.

### Naming rows and columns

This course uses `row`/`col` in early map code and `x`/`y` once entities
move. Both are fine; mixing them in one function is not. Prefer:

- `y` increases **downward** on screen (row 0 is the top), matching how
  terminals paint lines;
- `world[y][x]` with **y first** — that matches the declaration
  `char world[MAP_HEIGHT][MAP_WIDTH]` (rows outer, columns inner).

If you ever write `world[x][y]` "because x comes first in speech," you will
transpose the map and spend an hour staring at a sideways village.

## Apply it

Create a new module, `map.h` / `map.c`, following the same header/source
split from Chapter 4:

```c
/* map.h */
#ifndef MAP_H
#define MAP_H

#define MAP_HEIGHT 5
#define MAP_WIDTH  10

void map_print(void);

#endif
```

```c
/* map.c */
#include <stdio.h>
#include "map.h"

static char world[MAP_HEIGHT][MAP_WIDTH] = {
    { '#','#','#','#','#','#','#','#','#','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','.','.','#','#','#','#','.','.','#' },
    { '#','.','.','.','.','.','.','.','.','#' },
    { '#','#','#','#','#','#','#','#','#','#' }
};

void map_print(void)
{
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int col = 0; col < MAP_WIDTH; col++) {
            printf("%c", world[row][col]);
        }
        printf("\n");
    }
}
```

`world` is `static`, the same keyword Chapter 4 used to hide a function
inside its file — here it hides a *variable* the exact same way. Nothing
outside `map.c` can see `world` directly, or accidentally corrupt it; the
only way in is through the functions `map.c` chooses to expose, which
right now is just `map_print`. This is worth sitting with: `map_print`
takes no parameters and needs none, because the data it operates on isn't
passed in — it already lives inside the module that owns it. (Passing a
2D array *into* a function is possible in C, but it involves array-to-
pointer decay, which is squarely Chapter 7 territory — this module
structure is specifically chosen to avoid needing that yet.)

Add `map.c` to your `Makefile`'s `SOURCES`:

```make
SOURCES = main.c character.c map.c duel.c
```

And call `map_print` from `main.c`, between the character intro and the
duel:

```c
#include "map.h"
```

```c
character_intro();

printf("\n");
printf("The village gate creaks open.\n");
printf("\n");
map_print();

duel_run();
```

## Compile and run

```bash
make clean
make
./game
```

Expected output (name `Elowen`, calling `h`):

```
****************************************
*                                      *
*           UNTITLED RPG               *
*                                      *
****************************************

What is your name, hero? Elowen

Welcome, Elowen
----------------------------------------
HP:     20
MP:     8
ATK:    6
DEF:    4
Rank:   E
Power:  5.0
----------------------------------------

The village gate creaks open.

##########
#........#
#..####..#
#........#
##########

A stranger flips an old coin and grins.
Call it: (h)eads or (t)ails? h
The coin spins...
It lands on heads. You called it! You win the duel.
```

Zero warnings, as always.

## What just happened

`world[row][col]` looks like two independent lookups, but it's really one
computation: `row * MAP_WIDTH + col` gives a single offset into one
contiguous block of `MAP_HEIGHT * MAP_WIDTH` characters, and that's the
slot that gets read. A 2D array in C isn't a grid of separately-allocated
rows (some other languages do work that way) — it's one flat strip of
memory that you and the compiler agree to *think about* as rows and
columns. That's also exactly why `world[5][0]` (one row past the end)
would be undefined behaviour for the same reason `scores[5]` was: there's
no boundary check anywhere in this computation, at any dimension.

```
world (flat, MAP_HEIGHT * MAP_WIDTH = 50 chars):
[ # # # # # # # # # # | # . . . . . . . . # | # . . # # # # . . # | ... ]
  \______row 0_______/ \______row 1_______/ \______row 2_______/
```

## Common errors

**Off-by-one in a loop bound — reads one past the end:**

```c
for (int row = 0; row <= MAP_HEIGHT; row++) {   /* <= should be < */
    for (int col = 0; col < MAP_WIDTH; col++) {
        printf("%c", world[row][col]);
    }
    printf("\n");
}
```

This compiles with no warning and usually *runs* without an obvious crash
— it just silently reads (and prints) whatever ten bytes of memory happen
to sit right after `world`, as though they were a sixth row of the map.
`<=` instead of `<` on an array bound is one of the single most common
bugs in C, precisely because it compiles clean and often doesn't crash —
it just produces subtly wrong output, which is far more dangerous than an
obvious failure. Whenever a loop's job is "visit every valid index," triple
-check whether the bound should be `<` or `<=` against the array's actual
size, not against habit.

**Array size mismatch between declaration and initializer:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -c toolong.c -o /dev/null
toolong.c:5:54: warning: excess elements in array initializer
    5 |     char world[2][3] = { {'a','b','c'}, {'d','e','f','g'} };
      |                                                      ^~~
toolong.c:5:54: note: (near initialization for ‘world[1]’)
```

Unlike out-of-bounds *indexing*, this one the compiler does catch — the
initializer's shape is known completely at compile time, so `gcc` can
count the braces and elements directly and flag the extra `'g'` (row 1 was
declared to hold 3 characters and got 4). It's only a *warning*, though,
not a hard error — the code still compiles, with the excess value silently
discarded. Treat it exactly as seriously as any other warning in this
course: something is wrong, even though the compiler let it through.

## Exercises

> **Practice drills:** complete `code/ch05/practice/` (`01_row_major` through
> `04_find_tile`) before exercise 3. Standalone programs — engrain indexing
> before you keep editing the game map by hand.

1. Change the deliberate off-by-one in "Common errors" above (`<=` instead
   of `<` on the outer loop) and actually run it. What extra line of
   "map" appears at the bottom? Explain with the flat-memory diagram and
   the `row * MAP_WIDTH + col` formula from the spotlight.
2. *Practice (required) + small durable edit.* Finish all four drills in
   `code/ch05/practice/`. Then add a sixth row to `world` (update
   `MAP_HEIGHT` to `6`, exactly `MAP_WIDTH` characters) using `,` for
   grass — lasting content, not a throwaway experiment.
3. *Negative indices.* Drill-style: in a **standalone** program (or extend
   practice), print `array[-1]` on a 5-element `int` array. Does it
   compile? Crash? Print `array[0]`? What does that say about bounds
   checks in either direction? (Do not leave this in the game binary.)
4. *Open-ended:* This map is 10×5 — smaller than most terminals. Sketch
   what must change for a map *larger* than the screen (world vs visible
   window). Chapter 9 solves it; naming it now makes that chapter click.

<details>
<summary>Solutions</summary>

1. An extra line prints — whatever 10 bytes sit after `world`. On this
   toolchain often NULs (`cat -A` shows `^@`). Row index `MAP_HEIGHT` is
   one past the last valid row; offset `5 * 10 + col` lands outside the
   50-byte block. Undefined behaviour; your bytes may differ.
2. Drills must print `all checks passed`. For the row: keep `MAP_HEIGHT`
   in sync with initializer count; each row exactly `MAP_WIDTH` chars.
3. Compiles clean at course flags. Does not reliably print `array[0]` —
   negative indices address *before* the array. No bounds checking either
   direction. See also practice `02_clamp_index` for the defensive habit.
4. You need world coordinates for the full grid *and* a moving visible
   window (camera). Chapter 9.

</details>

## Next up

Right now the map is data with no player on it, and the character sheet is
five separate loose variables (`hp`, `mp`, `attack`...) instead of one
coherent "hero." Chapter 6 introduces `struct` to bundle related data
together, and draws your hero as an actual position on the map for the
first time.
