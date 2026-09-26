# Chapter 11: Reading Files

## Where we are

Chapter 10 moved the map onto the heap: `map_create` allocates, the game
owns it, `map_destroy` releases it, and the whole thing runs clean under
the sanitizers. But the map *data* is still twenty string literals sitting
in `map.c`. Changing a single wall means editing C source and rebuilding
the entire program. This chapter cuts that cord: maps move into a text
file, and `map_load` reads them at runtime.

## The problem

Reading a file is the first time your program takes input from something
you don't control. Every previous input came from a keyboard, immediately,
with a human watching. A file can be missing, empty, truncated, full of
characters you never expected, or claim to be a size it isn't. It can also
be *edited by you at 2am* into a shape you forgot was possible. The
central discipline of this chapter is not `fopen` — it's the assumption
underneath every line: **input lies, and the program that trusts it
crashes.**

## C concept spotlight

### `FILE *`, `fopen`, `fclose`

```c
#include <stdio.h>

FILE *f = fopen("assets/maps/overworld.map", "r");
if (f == NULL) {
    fprintf(stderr, "cannot open the file\n");
    return NULL;
}

/* ... read from f ... */

fclose(f);
```

`fopen` returns a `FILE *` — a pointer to a **stream**, the standard
library's bookkeeping for an open file (where you are in it, buffered
data not yet handed to you, whether an error has occurred). You never
inspect a `FILE` yourself; you pass the pointer to functions that do.

The second argument is the **mode**: `"r"` for reading text, `"w"` for
writing (which truncates an existing file to nothing — be careful),
`"a"` for appending. This chapter only reads.

`fopen` returns `NULL` when it fails, and it fails *routinely* — wrong
path, missing file, no permission. That check is not optional, and it is
the same discipline as checking `malloc`: an unchecked `NULL` here becomes
a dereference and a crash later, further from the actual mistake.

`fclose` releases the stream. An open file is a resource with an owner,
exactly like a heap allocation — and just like `malloc`/`free`, every
`fopen` needs a matching `fclose` on *every* path out of the function,
including the error paths. You'll see that cost below, and it's real.

### `fgets` for lines

You met `fgets` back in Chapter 2, reading from `stdin`. It works the same
way on a file, because `stdin` *is* a `FILE *`:

```c
char line[MAX_LINE];

while (fgets(line, sizeof line, f) != NULL) {
    /* one line at a time */
}
```

`fgets` reads at most `sizeof line - 1` bytes, stops at a newline, and
adds a terminating byte. It returns `NULL` at end-of-file or on error —
which is how the loop above knows to stop. Two properties matter here:

- **It cannot overflow your buffer**, because you told it the size. This
  is why this course uses `fgets` and never `gets` (which was removed from
  the language for exactly this reason) or unbounded `scanf("%s")`.
- **It keeps the newline** in the buffer when the line fits. You almost
  always have to trim it, and forgetting to is a classic source of
  off-by-one comparisons that fail for no visible reason.

### `sscanf` for parsing a line you already have

To pull two integers out of a header line like `40 20`:

```c
int width = 0;
int height = 0;
if (sscanf(line, "%d %d", &width, &height) != 2) {
    /* the line did not contain two integers */
}
```

`sscanf` parses from a string in memory (the `ss` is "string scan"),
rather than from a stream. It returns **the number of items it
successfully converted** — and that return value is the entire safety
mechanism. Asking for two and getting two means both parsed; anything else
means the line wasn't what you expected, and you must not use the
variables.

Note the `&width` and `&height`: `sscanf` needs to *write* to your
variables, so it needs their addresses — Chapter 7's pass-by-reference,
showing up in a library function. This is also why `sscanf` is easy to get
catastrophically wrong: pass a value where an address belongs and the
compiler may not stop you, but the write lands somewhere arbitrary.

### Validating everything

Here's the mindset shift. A file is not data — it's a *claim* about data.
The header claims a width and height. The rows claim to be that wide.
The characters claim to be tiles. Every one of those claims needs
checking before it's used to index memory:

| The file says | What could go wrong | Check |
|---|---|---|
| `40 20` | missing, or not two numbers | `sscanf(...) != 2` |
| width `0`, or `999999` | zero-size or absurd allocation | range check |
| a row of tiles | too short, too long | compare length to width |
| a tile character | `X` isn't a tile | whitelist valid tiles |
| 20 rows follow | file ends after 12 | check `fgets` each row |

Skip any of these and the failure doesn't happen at the check — it happens
later, in `map_tile_at`, indexing past the end of an allocation that
turned out smaller than the header promised. That's the Chapter 5
out-of-bounds read again, arriving from a completely different direction.

## Apply it

### The map file format

Create `assets/maps/overworld.map`. The first line is `width height`;
every following line is one row of exactly `width` tiles:

```
40 20
########################################
#......#################...............#
#......#################...#########...#
#......#####........####...#.......#...#
#............#######.......#.......#...#
#......#####........####...#...#####...#
#......#################...#...........#
#......#################...#############
#..........................#############
############...#############..........##
############...#############..........##
#..............#############..........##
#...####################...............#
#...####################...#########...#
#...................####...#########...#
#########..............................#
#########...####################...#####
#.......................########...#####
#.......................########.......#
########################################
```

This is exactly the map from Chapters 9 and 10 — the same data, now
living somewhere you can edit it.

### `map.h` — `map_create` becomes `map_load`

```c
/* Paired lifetime: every map_load must be matched by exactly one
   map_destroy. The caller owns the returned Map and is responsible for
   destroying it.

   Returns NULL if the file cannot be opened, is malformed, or memory
   runs out. On failure, an explanation is printed to stderr and nothing
   is left allocated. */
Map *map_load(const char *path);
void map_destroy(Map *map);
```

That second paragraph is the important half of the contract. "Returns
`NULL` on failure" is not enough information for a caller — they also need
to know whether a failed call left anything for them to clean up. Here it
doesn't, and saying so explicitly is what makes the function safe to use
without reading its body.

### `map.c` — the loader

The top of the file, replacing the twenty string literals:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

/* Longest line we will accept: a map row, plus a newline, plus the
   terminating NUL, plus a little slack so we can detect over-long rows
   instead of silently truncating them. */
#define MAX_LINE 1024

/* Sanity limits. A map outside these bounds is treated as malformed
   rather than trusted -- a file on disk is input, and input lies. */
#define MAX_DIMENSION 4096

static int is_valid_tile(char c)
{
    return c == '#' || c == '.';
}
```

Opening the file and parsing the header:

```c
Map *map_load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "map_load: cannot open '%s'\n", path);
        return NULL;
    }

    char line[MAX_LINE];

    if (fgets(line, sizeof line, f) == NULL) {
        fprintf(stderr, "map_load: '%s' is empty\n", path);
        fclose(f);
        return NULL;
    }

    int width = 0;
    int height = 0;
    if (sscanf(line, "%d %d", &width, &height) != 2) {
        fprintf(stderr, "map_load: '%s' has no valid 'width height' header\n",
                path);
        fclose(f);
        return NULL;
    }

    if (width <= 0 || height <= 0 ||
        width > MAX_DIMENSION || height > MAX_DIMENSION) {
        fprintf(stderr, "map_load: '%s' has unreasonable dimensions %dx%d\n",
                path, width, height);
        fclose(f);
        return NULL;
    }
```

Notice `fclose(f)` appearing on every single failure path. That
repetition is the honest cost of C's lack of automatic cleanup — there's
no `finally` block and no destructor running on scope exit. Miss one and
you leak a file handle: a real resource, in limited supply, and one that
sanitizers won't warn you about the way they would for `malloc`.

Allocating, with the same paired-failure care as Chapter 10:

```c
    Map *map = malloc(sizeof *map);
    if (map == NULL) {
        fprintf(stderr, "map_load: out of memory\n");
        fclose(f);
        return NULL;
    }

    map->width = width;
    map->height = height;

    map->tiles = malloc((size_t)width * (size_t)height);
    if (map->tiles == NULL) {
        fprintf(stderr, "map_load: out of memory\n");
        free(map);
        fclose(f);
        return NULL;
    }
```

Reading and validating each row — the heart of the function:

```c
    for (int y = 0; y < height; y++) {
        if (fgets(line, sizeof line, f) == NULL) {
            fprintf(stderr, "map_load: '%s' ended early (row %d of %d)\n",
                    path, y, height);
            map_destroy(map);
            fclose(f);
            return NULL;
        }

        /* fgets keeps the newline; trim it (and a \r, for files that
           came from Windows) before measuring the row. */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        if (len != (size_t)width) {
            fprintf(stderr,
                    "map_load: '%s' row %d is %zu tiles, expected %d\n",
                    path, y, len, width);
            map_destroy(map);
            fclose(f);
            return NULL;
        }

        for (int x = 0; x < width; x++) {
            if (!is_valid_tile(line[x])) {
                fprintf(stderr,
                        "map_load: '%s' row %d has invalid tile '%c'\n",
                        path, y, line[x]);
                map_destroy(map);
                fclose(f);
                return NULL;
            }
        }

        memcpy(map->tiles + (size_t)y * (size_t)width, line, (size_t)width);
    }

    fclose(f);
    return map;
}
```

Three details worth pausing on:

**`map_destroy(map)` on the row-loop failure paths.** By this point both
allocations have succeeded, so bailing out means releasing them — and
because `map_destroy` already knows how to free a `Map` correctly, you
reuse it rather than writing `free(map->tiles); free(map);` five times.
This is why `map_destroy` tolerating `NULL` (Chapter 10) pays off: cleanup
code stays uniform.

**The `\r` in the newline trim.** A map file created on Windows ends its
lines with `\r\n`, not `\n`. Without trimming `\r`, every row would
measure one byte too long and be rejected as malformed — with a confusing
message, since the row *looks* right in an editor. Given this course runs
under WSL with a Windows host one filesystem away, this is not a
hypothetical.

**`%zu` for `strlen`'s result.** `strlen` returns `size_t`, an unsigned
type whose exact width varies by platform, and `%zu` is its matching
format specifier. Using `%d` here would be the Chapter 2 format-mismatch
bug — which `-Wformat` would catch, but knowing the right specifier beats
being corrected by the compiler.

### Wiring it up

In `main.c`, one line changes:

```c
Map *map = map_load("assets/maps/overworld.map");
if (map == NULL) {
    fprintf(stderr, "Could not load the map. See the message above.\n");
    return 1;
}
```

`map_load` has already printed the specific problem, so `main` only needs
to say that it's fatal and stop. Note the path is relative to wherever you
run `./game` from — this course assumes you run it from the project
directory, which is what `make run` does.

## Compile and run

```bash
make clean
make
./game
```

The game plays exactly as before, because the map data is unchanged. Now
the actual payoff — **edit `assets/maps/overworld.map` in any text
editor**, change some tiles in a row (keeping the row exactly 40
characters, using only `#` and `.`), save, and run `./game` again
**without rebuilding**:

```bash
./game          # no make, no compiler, no linker
```

The change is there. Verified — replacing row 1 with a `#.#.#.` pattern
and running the untouched binary shows it immediately:

```
#.#.#.#.#.#.#.#.#.#.
```

That's the whole point of this chapter. Content and code are now separate
things, on separate schedules, editable by separate people.

Then check the failure paths, which are just as important as the success
one:

```bash
./game < /dev/null                    # after temporarily renaming the map file
```

```
map_load: cannot open 'assets/maps/overworld.map'
Could not load the map. See the message above.
```

And the sanitizer run, required from Chapter 10 onward:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

Silence, as before.

## What just happened

`map_load` performs a sequence of narrowing claims, each verified before
the next is trusted: the file exists → it has a first line → that line
holds two integers → those integers are plausible sizes → memory for them
is available → each of `height` rows exists → each is exactly `width`
characters → each character is a legal tile. Only after all of that does
any of the data reach `map->tiles`.

```
  overworld.map                      map_load                    HEAP
+-----------------+                                      +----------------+
| "40 20"         | --sscanf--> width=40, height=20 ---> | Map{40,20,...} |
| "###...####"    | --fgets---> trim \n, check len 40 -> |                |
| "#...#...#"     |             check each tile char     | tiles: 800 B   |
|      ...        |             memcpy row into tiles -> | "###...####..." |
+-----------------+                                      +----------------+
       ^                                                          ^
   a claim                                                   a fact
```

The failure paths all converge on the same shape: print what's wrong,
release whatever has been acquired so far (heap, then file), return
`NULL`. Getting that release order and completeness right on *every*
branch is most of the work in real C I/O code, and it's why the function
is long relative to what it does.

## Common errors

**Forgetting to check `fopen`'s return:**

```c
FILE *f = fopen(path, "r");
char line[128];
fgets(line, sizeof line, f);     /* f may be NULL */
```

```
$ ./game
Segmentation fault (core dumped)
```

Passing `NULL` to `fgets` is undefined behaviour, and in practice it
crashes immediately with no indication that a *missing file* was the real
problem. The crash is in `fgets`; the bug is three lines earlier.

**Forgetting to trim the newline:**

```c
size_t len = strlen(line);
if (len != (size_t)width) { /* rejects every single row */ }
```

```
map_load: 'assets/maps/overworld.map' row 0 is 41 tiles, expected 40
```

Every row is one byte longer than it looks, because `fgets` kept the `\n`.
The message is at least honest about the count, which makes this one of
the friendlier bugs in the chapter — but if you'd instead `memcpy`'d
`width` bytes without checking, the newline would have silently become a
tile and never been noticed.

**Trusting the header's dimensions:**

```c
/* no range check on width/height */
map->tiles = malloc((size_t)width * (size_t)height);
```

A file claiming `999999 999999` requests roughly a terabyte; `malloc`
returns `NULL`, and if you also skipped *that* check, the next write
crashes. A file claiming `0 0` allocates zero bytes — `malloc(0)` is legal
and returns a pointer you must not dereference, so the failure is silent
and arrives later. And a negative width, converted to `size_t`, becomes an
enormous positive number. All three are why `map_load` range-checks before
allocating, not after.

**Losing the file handle on an error path:**

```c
if (sscanf(line, "%d %d", &width, &height) != 2) {
    return NULL;                 /* forgot fclose(f) */
}
```

Nothing crashes, no sanitizer complains, and the program works fine — this
is a resource leak, not a memory error, and ASan does not track file
handles. In a program that opens one map at startup, it's invisible. In
one that loads a map every time you enter a building, it eventually
exhausts the process's file-descriptor limit and every subsequent `fopen`
fails for no apparent reason. Leaks you can't detect are the ones worth
being systematic about.

## Exercises

> **Practice drills:** work `code/ch11/practice/01-map-fixtures/` (`01_header`,
> `02_row_width`, `03_skip_comments`) before exercise 2. Do **not** corrupt
> shipped `assets/maps/` for negative cases — use the fixtures in that folder.
> Hostile-input checks against the real loader stay valuable: point it at a
> *copy* of a fixture path if you want, never at production `overworld.map`.

1. *Hostile fixtures (practice).* Complete `01-map-fixtures` (`make && make check`).
   Open each `fixtures/*.map` and predict the diagnostic before you run. Confirm
   clear failures on bad header, short row, invalid tile, truncated grid, and
   absurd height — using **fixtures**, not the shipped overworld.
2. *Durable format feature.* After the practice folder, add `;` comment-line
   skipping to real `map_load` (lasting). Do not advance the row counter for
   comment lines.
3. Sketch `MapLoadResult` (or an out-param enum) so callers can tell "missing"
   from "malformed" without scraping stderr. Optional implement.
4. *Open-ended:* Design a start-position extension that keeps old maps
   loading. What changes in the parse loop?

<details>
<summary>Solutions</summary>

1. See `code/ch11/practice/01-map-fixtures/SOLUTION.md` and `solutions/`.
   Fixture matrix: `bad_header`, `short_row`, `invalid_tile`, `truncated`,
   `absurd_height`, plus `ok` / `comments`.
2. In `map_load`, after `fgets`, skip lines whose first non-space char is `;`
   — and do **not** advance the row counter for those lines. Practice
   `03_skip_comments` is the rehearsal.
3. `typedef enum { MAP_LOAD_OK, MAP_LOAD_MISSING, MAP_LOAD_BAD_HEADER, ... } MapLoadResult;`
   with `Map *map_load(const char *path, MapLoadResult *out);`
4. A second header line, or a `start x y` record after the grid; version
   or optional lines keep old files working.

</details>

## Next up

Your map loader manipulates text constantly — measuring lines, trimming
newlines, copying bytes — using functions like `strlen` and `memcpy` that
haven't been properly explained yet. Chapter 12 fixes that: how C strings
actually work, why they end in a NUL byte, how they overflow, and what it
takes to wrap a paragraph of NPC dialogue into a text box.
