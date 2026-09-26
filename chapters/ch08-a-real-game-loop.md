# Chapter 8: A Real Game Loop

## Where we are

Your hero walks and bumps into walls — real pointer-driven mutation,
verified working. But every move still requires typing a letter *and*
pressing Enter, and the whole "game" is really a scripted sequence of
`printf`/`fgets` calls from top to bottom. This chapter changes both: a
real display library takes over the screen, keys react the instant you
press them, and — just as importantly — the rest of your code never has to
know or care that it's `ncurses` doing the drawing.

## The problem

`printf` and `fgets` are line-oriented: they only see input after Enter,
and they only ever add text to the bottom of a scrolling terminal. A real
game needs to redraw the *whole screen* every frame and react to *single
keys instantly*. That's a job for a dedicated library — this course uses
**ncurses**, chosen at Checkpoint A — but wiring your game logic directly
to ncurses function calls everywhere would mean every future chapter's
code is permanently tied to one specific library. You need a wall between
"what the game wants to draw" and "how it actually gets drawn."

## C concept spotlight: opaque interfaces

You've built `.h`/`.c` pairs since Chapter 4 — a header declaring what
exists, a source file defining how. This chapter uses that same mechanism
for a more deliberate purpose: an **opaque interface**, where the header
is written so that *nothing* about the underlying implementation leaks
through it.

Compare these two possible designs for "draw a tile on screen":

```c
/* Leaky: exposes ncurses' own types directly */
void render_draw_tile(WINDOW *win, int x, int y, chtype ch);
```

```c
/* Opaque: no hint that ncurses is involved at all */
void render_draw_tile(int x, int y, TileType t);
```

The second version's signature is built entirely from types your game
already understands — `int` and a `TileType` enum you define yourself.
Nothing here reveals that `render_draw_tile` happens to call an ncurses
function called `mvaddch` internally. That's the point: any code calling
`render_draw_tile` compiles and runs identically whether the file behind
`render.h` is `render_ncurses.c` (what you write this chapter) or, someday,
`render_sdl.c` — a completely different library, opening an actual
graphical window instead of drawing to a terminal. Swapping the `.c` file
and updating one line in the `Makefile` would be enough; not one line of
your game logic would need to change, because none of it ever `#include`d
`<ncurses.h>` in the first place.

This is the same idea as Chapter 4's `static` (hide what other files don't
need), aimed upward instead of inward: hide *which library* does the work,
so the rest of your program only ever depends on the shape of the
interface, never the implementation behind it.


### Why the present seam stays

`render_present` is a one-line wrapper today. That is not a reason to
delete it. The call site in `draw_world` says "frame is ready" in *game*
vocabulary; `refresh()` says "ncurses, flush now." When Chapter 24 (or an
SDL port) swaps backends, every `refresh()` buried in game code becomes a
hunt. Practice drill `02-render-boundary` makes the failure mode obvious
in twenty lines — run that instead of gutting the live binary.

## Apply it

### The rendering interface

`render.h` — the only thing the rest of the game will know about drawing,
from here forward:

```c
#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>

typedef enum {
    TILE_FLOOR,
    TILE_WALL,
    TILE_PLAYER
} TileType;

bool render_init(void);
void render_shutdown(void);
void render_clear(void);
void render_draw_tile(int x, int y, TileType t);
void render_draw_text(int x, int y, const char *s);
void render_present(void);

#endif
```

`bool` comes from `<stdbool.h>` — a real boolean type, `true`/`false`,
that this course hasn't used until now (everywhere else, `0`/`1` on a
plain `int` did the job, by convention rather than by the type system).
`render_init` uses it because "did the display start up successfully?" is
exactly the kind of genuinely two-valued question `bool` was made for.

`render_ncurses.c`:

```c
#include <ncurses.h>
#include "render.h"

bool render_init(void)
{
    if (initscr() == NULL) {
        return false;
    }

    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    return true;
}

void render_shutdown(void)
{
    endwin();
}

void render_clear(void)
{
    clear();
}

void render_draw_tile(int x, int y, TileType t)
{
    char symbol = '?';

    switch (t) {
        case TILE_FLOOR:  symbol = '.'; break;
        case TILE_WALL:   symbol = '#'; break;
        case TILE_PLAYER: symbol = '@'; break;
    }

    mvaddch(y, x, (chtype)symbol);
}

void render_draw_text(int x, int y, const char *s)
{
    mvprintw(y, x, "%s", s);
}

void render_present(void)
{
    refresh();
}
```

Four ncurses calls do all the real work, each wrapped so nothing outside
this file needs to know their names:

- `initscr()` hands ncurses the terminal; `NULL` back means it failed
  (wrong `$TERM`, no terminal at all).
- `noecho()` stops typed characters from appearing on screen automatically
  — your game decides what gets drawn, not the terminal.
- `cbreak()` is the single most important call in this chapter: it puts
  the terminal into **raw-ish input mode**, where a keypress becomes
  readable *the instant it's typed*, instead of only after Enter (which is
  called "canonical mode," and is what every previous chapter's
  `fgets`/`getchar` calls have relied on all along).
- `curs_set(0)` hides the blinking text cursor, which would otherwise
  distractingly hover wherever ncurses last drew something.
- `mvaddch(y, x, ...)` moves to row `y`, column `x`, and draws one
  character — note the argument order is row-then-column, the opposite of
  the `(x, y)` order this course's own functions use, which is exactly why
  wrapping it behind `render_draw_tile(x, y, ...)` is worth doing: callers
  never have to remember ncurses' quirk.
- `refresh()` is what actually pushes every queued change to the visible
  screen at once — ncurses batches drawing calls internally and only
  updates the terminal when you ask it to, which is far more efficient
  than redrawing after every single change.

### The input interface

`input.h`:

```c
#ifndef INPUT_H
#define INPUT_H

typedef enum {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_QUIT
} InputEvent;

InputEvent input_poll(void);

#endif
```

`input.c`:

```c
#include <ncurses.h>
#include "input.h"

InputEvent input_poll(void)
{
    int key = getch();

    switch (key) {
        case 'w': case 'W': return INPUT_UP;
        case 's': case 'S': return INPUT_DOWN;
        case 'a': case 'A': return INPUT_LEFT;
        case 'd': case 'D': return INPUT_RIGHT;
        case 'q': case 'Q': return INPUT_QUIT;
        default: return INPUT_NONE;
    }
}
```

`getch()` is ncurses' own key-reading function — thanks to `cbreak()` from
`render_init`, it returns the instant a key is pressed, with no Enter and
no leftover-newline cleanup needed at all. Compare this to Chapter 7's
`read_command`, which needed a whole loop just to discard everything after
the first character — `getch()` makes that entire problem disappear,
because it never puts the terminal into line-buffered mode to begin with.
`input_poll` translates raw key codes into `InputEvent` values for exactly
the same reason `render.h` hides `ncurses.h`: nothing outside this file
needs to know that movement happens to be WASD-mapped, or that ncurses
calls it `getch`.

### Wiring it into `main.c`

```c
#include <stdio.h>
#include "entity.h"
#include "map.h"
#include "render.h"
#include "input.h"
#include "duel.h"

static void draw_world(const Player *player)
{
    render_clear();

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            TileType t = map_is_walkable(x, y) ? TILE_FLOOR : TILE_WALL;
            render_draw_tile(x, y, t);
        }
    }

    render_draw_tile(player->x, player->y, TILE_PLAYER);
    render_draw_text(0, MAP_HEIGHT + 1, "w/a/s/d to move, q to quit wandering");

    render_present();
}

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*           UNTITLED RPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");

    Player player = entity_create_player();
    entity_print_sheet(&player);

    printf("\n");
    printf("The village gate creaks open. Press Enter to step through...\n");
    getchar();

    if (!render_init()) {
        printf("Could not start the display. Is $TERM set correctly?\n");
        return 1;
    }

    InputEvent event = INPUT_NONE;
    while (event != INPUT_QUIT) {
        draw_world(&player);

        event = input_poll();

        int dx = 0;
        int dy = 0;

        switch (event) {
            case INPUT_UP:    dy = -1; break;
            case INPUT_DOWN:  dy = 1;  break;
            case INPUT_LEFT:  dx = -1; break;
            case INPUT_RIGHT: dx = 1;  break;
            case INPUT_QUIT:  break;
            case INPUT_NONE:  break;
        }

        if (dx != 0 || dy != 0) {
            int target_x = player.x + dx;
            int target_y = player.y + dy;

            if (map_is_walkable(target_x, target_y)) {
                entity_move(&player, dx, dy);
            }
        }
    }

    render_shutdown();

    duel_run();

    return 0;
}
```

Notice `map_is_walkable` from Chapter 7 needed no changes at all —
`map.c` never knew or cared that a new rendering system arrived; it just
answers "is this tile walkable," same as always. That's the interface
boundary working exactly as intended. Also notice character creation
(`entity_create_player`) and the duel (`duel_run`) still use plain
`printf`/`fgets`/`getchar`, running entirely *before* `render_init()` and
*after* `render_shutdown()` — mixing ncurses drawing with ordinary
terminal output while ncurses has control produces garbled output, so
those two boundaries matter. One honest gap: the wall-bump message from
Chapter 7 is gone for now — there's no scrolling text log yet to put it
in, so a blocked move currently just does nothing visibly. That's a real
regression in feedback, and it's staying that way until Chapter 12 gives
this game a proper text box to report things like that in.

Update the `Makefile` to compile the two new files and link the ncurses
library:

```make
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g
LDLIBS = -lncursesw

SOURCES = main.c entity.c map.c duel.c render_ncurses.c input.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = game

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
```

The only change from Chapter 4's `Makefile` is `LDLIBS = -lncursesw` (the
wide-character build of ncurses, which is what Arch's `ncurses` package
provides) added to the two new source files and passed on the link line —
libraries need to be listed when *linking*, not when compiling each
individual `.c` file, which is why `LDLIBS` only appears on the
`$(TARGET)` rule's recipe line, not the pattern rule above it.

## Compile and run

```bash
make clean
make
./game
```

Enter a name, press Enter through the character sheet, then press Enter
once more at the gate — and from that point on, `w`/`a`/`s`/`d` move you
instantly, no Enter required, and `q` exits back to a normal terminal and
starts the duel. You should see something like this fill your terminal
(the `w/a/s/d...` line sits below the map, and `@` moves as you press
keys):

```
##########
#@.......#
#..####..#
#........#
##########
w/a/s/d to move, q to quit wandering
```

Zero warnings from `make`.

## What just happened

Before `render_init()` runs, your terminal is in **canonical mode**: the
terminal driver itself buffers a whole line and only hands it to your
program once Enter is pressed — this is why every chapter before this one
needed Enter, with no exceptions, no matter how the input was read. Inside
`render_init()`, `cbreak()` reprograms that terminal driver setting
directly, telling it to hand over each keypress immediately instead.
`endwin()` (called by `render_shutdown`) puts canonical mode back exactly
the way it found it — which is why `duel_run()`'s `getchar()`-based input
works completely unchanged afterward, needing Enter again just like every
earlier chapter.

```
Before render_init()            After render_init()           After render_shutdown()
 (canonical mode)                (cbreak mode)                 (canonical mode, restored)
+--------------------+          +------------------+          +--------------------+
| keypress -> OS     |          | keypress -> OS   |          | keypress -> OS     |
| buffers whole line |          | delivers key     |          | buffers whole line |
| waits for Enter    |          | to program       |          | waits for Enter    |
| THEN hands to      |          | INSTANTLY        |          | THEN hands to      |
| your program       |          |                  |          | your program       |
+--------------------+          +------------------+          +--------------------+
```

## Common errors

**`initscr` fails because `$TERM` isn't set (or is set to something
ncurses doesn't recognize):**

```
$ TERM=unknown ./game
ncurses: cannot initialize terminal type ($TERM="unknown"); exiting
```

Here's something worth knowing honestly rather than glossing over:
`render_init`'s `if (initscr() == NULL)` check was written expecting
`initscr` to hand failure back so *your* code could report it and exit
cleanly. On this toolchain, ncurses doesn't do that for an unrecognized
`$TERM` — it prints its own message and calls `exit()` immediately, from
inside `initscr` itself, before ever returning to `render_init` at all.
The `NULL` check isn't wrong to have (some ncurses builds, and other
failure modes, really do return `NULL` instead), but don't assume it's
your safety net for *every* way `initscr` can fail — this is a good early
lesson that library documentation ("returns `NULL` on failure") and a
specific library's actual behavior on your system aren't always the same
thing, and the only way to know for sure is to check, the way this chapter
just did. If you hit this for real, check `echo $TERM` — Chapter 0's
Appendix A troubleshooting entry covers what a sane value looks like.

**Forgetting `-lncursesw` in the link step:**

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o game main.o entity.o map.o duel.o render_ncurses.o input.o
/usr/bin/ld: render_ncurses.o: in function `render_init':
render_ncurses.c:(.text+0x5): undefined reference to `initscr'
/usr/bin/ld: render_ncurses.c:(.text+0x16): undefined reference to `noecho'
/usr/bin/ld: render_ncurses.c:(.text+0x1b): undefined reference to `cbreak'
/usr/bin/ld: render_ncurses.c:(.text+0x25): undefined reference to `curs_set'
/usr/bin/ld: render_ncurses.c:(.text+0x2c): undefined reference to `stdscr'
/usr/bin/ld: render_ncurses.c:(.text+0x39): undefined reference to `keypad'
/usr/bin/ld: render_ncurses.o: in function `render_shutdown':
render_ncurses.c:(.text+0x49): undefined reference to `endwin'
[... one more "undefined reference" for every remaining ncurses
    function used, across render_ncurses.o and input.o ...]
collect2: error: ld returned 1 exit status
```

Another `undefined reference` error, exactly Chapter 4's category — except
this time the missing definitions aren't a file you forgot to compile,
they're an entire external library ncurses ships as a compiled `.so` file,
not source you built yourself. Every single ncurses function your code
calls shows up as its own undefined reference, which is why the real
output here is much longer than the sample above — trimmed for length,
but each line has the identical shape. `-lncursesw` tells the linker "go
look inside the ncurses library for whatever's still undefined" — without
it, the linker has no idea any of these functions exist anywhere.

**Leftover screen garbage after the program exits unexpectedly:**

If `./game` ever crashes or gets killed while ncurses has control (before
`render_shutdown()` runs), your terminal can be left in a strange state —
no visible cursor, or garbled-looking output on every subsequent command.
This isn't a bug in your code so much as an unavoidable risk of raw mode:
if `endwin()` never runs, canonical mode never gets restored. Run `reset`
(a real, separate terminal command, not part of this course's game) to
force your terminal back to sanity if this happens to you.

## Exercises

> **Practice drills:** work `code/ch08/practice/` in order (`01-glyph-table`,
> `02-render-boundary`, `03-key-dispatch`) before exercise 2. Do **not**
> delete `render_present` from the game or call `refresh()` from
> `draw_world` — that experiment belongs only in `02-render-boundary`.

1. *Glyph table (practice).* In `01-glyph-table`, implement `glyph_for`
   including `TILE_WATER`. Optionally flip wall to `%` in the **drill
   only**, then restore. Confirm you understand why live
   `render_draw_tile` centralizes the same decision.
2. *Durable UX — arrow keys.* Add `KEY_UP` / `KEY_DOWN` / `KEY_LEFT` /
   `KEY_RIGHT` to real `input_poll` (keypad already enabled). Use
   `03-key-dispatch` to rehearse the table first if you want.
3. *Seam check (practice, not a game edit).* Complete `02-render-boundary`
   so `draw_world` calls `render_present` twice across two frames. In your
   own words: what is lost if game code calls the backend refresh
   directly, even when behaviour matches today?
4. *Open-ended:* Sketch which sheet field (HP, gold, …) should stay on
   screen while wandering, and where `render_draw_text` would go. Implement
   later when string formatting is comfortable (Chapter 12); the lasting
   idea is a HUD call site, not a one-off `printf`.

<details>
<summary>Solutions</summary>

1. See `01-glyph-table/SOLUTION.md`. Live renderer: one `switch` or table
   in `render_ncurses.c` — callers pass `TileType` only.
2. ```c
   case 'w': case 'W': case KEY_UP:    return INPUT_UP;
   /* ... same pattern for down/left/right ... */
   ```
3. See `02-render-boundary/SOLUTION.md`. You lose the opaque boundary:
   game files start including backend headers and a second backend means
   editing every present call site.
4. HP is the natural first HUD line; draw after the map, at
   `VIEW_HEIGHT + 1` or similar, once you own `snprintf`.

</details>

## Next up

The map right now is small enough to fit entirely on screen at once. Real
RPG worlds aren't. Chapter 9 introduces the camera: a moving window onto
a world larger than your terminal, using modular arithmetic and clamping
to decide exactly what's visible at any moment.
