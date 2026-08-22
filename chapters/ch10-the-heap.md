# Chapter 10: The Heap

## Where we are

Your 40×20 world scrolls under a clamped camera, and everything the game
knows lives in variables whose sizes were fixed when you compiled. That's
been fine — but it means every map you ever add is baked into the
executable, at a size chosen by you, in advance, forever. This chapter
introduces the heap: memory you request while the program is *running*,
in whatever amount you decide then. It also introduces the two tools that
make manual memory management survivable, because you are about to write
your first memory bugs, on purpose, and find them.

## The problem

`static const char world[20][41]` has three limitations you can't
negotiate away: its size is a compile-time constant, it exists for the
entire life of the program whether you need it or not, and there's exactly
one of it. A real JRPG loads a map when you enter an area and releases it
when you leave, with maps of different sizes, several of them, chosen at
runtime. All of that requires asking the operating system for memory
during execution — and taking on the responsibility of giving it back.

## C concept spotlight

### The stack and the heap

Every variable you've written so far lives on the **stack**: memory the
compiler manages automatically. When a function is entered, its locals get
space; when it returns, that space is reclaimed, instantly and without
your involvement. That's why Chapter 6's `entity_move(Player p, ...)`
discarded its changes — `p` was a stack variable that ceased to exist at
`return`.

The **heap** is the other pool: memory you request explicitly, that lives
until you explicitly release it, regardless of which functions enter or
return in between.

```
     STACK                              HEAP
  (automatic)                       (yours to manage)
+---------------+                +-------------------+
| main's locals |                |                   |
|   map ------------------------>| Map { w, h, tiles}|
|   player      |                |         |         |
+---------------+                |         v         |
| draw_world's  |                | 800 bytes of tiles|
|   locals      |                |                   |
+---------------+                +-------------------+
  freed on return                  freed only by free()
```

### `malloc` and `free`

```c
#include <stdlib.h>

int *scores = malloc(3 * sizeof *scores);
if (scores == NULL) {
    fprintf(stderr, "out of memory\n");
    return 1;
}

scores[0] = 10;
scores[1] = 20;
scores[2] = 30;

free(scores);
```

`malloc` takes a number of **bytes** and returns a pointer to that much
uninitialized heap memory — or `NULL` if the request can't be satisfied.
Three details worth getting right from the first line you write:

- **`sizeof *scores`, not `sizeof(int)`.** Both are `4` here, but
  `sizeof *scores` means "the size of whatever `scores` points at," so it
  stays correct automatically if you later change `scores` to a different
  type. It reads oddly at first — you're applying `sizeof` to a
  dereference of a pointer that isn't valid yet — but `sizeof` never
  actually evaluates its operand, it only inspects its type at compile
  time. This is the idiom to build the habit around.
- **Always check for `NULL`.** This course checks every single
  allocation, with no exceptions and no "error handling omitted for
  brevity." A failed `malloc` that you don't check produces a `NULL`
  pointer that you then dereference — the guaranteed crash from Chapter 7.
- **`malloc` does not zero the memory.** What you get is whatever was
  there before. Reading it before writing it is undefined behaviour, in
  the same family as Chapter 5's out-of-bounds reads. (`calloc` zeroes;
  this course uses `malloc` and initializes deliberately.)

`free` returns the memory. After that call, `scores` still holds the same
address, but that address no longer belongs to you — using it is a
**dangling pointer**, a pointer to memory that has been released.

### Ownership as a discipline

The rule this course follows everywhere, starting now: **every allocation
has exactly one owner, and the owner is responsible for freeing it.**
The convention that makes ownership visible is paired lifetimes —
`foo_create` allocates, `foo_destroy` releases, and the header says so:

```c
/* Paired lifetime: every map_create must be matched by exactly one
   map_destroy. The caller owns the returned Map and is responsible for
   destroying it. Returns NULL if allocation fails. */
Map *map_create(void);
void map_destroy(Map *map);
```

That comment is not decoration. In a language with no garbage collector,
the *only* thing preventing leaks and double-frees is a clear, written
answer to "who frees this?" — write it down at the point of definition,
every time.

### Bug 1: the leak

Here is a program with a real bug. Compile and run it normally:

```c
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *scores = malloc(3 * sizeof *scores);
    if (scores == NULL) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    scores[0] = 10;
    scores[1] = 20;
    scores[2] = 30;

    printf("%d %d %d\n", scores[0], scores[1], scores[2]);

    /* forgot: free(scores); */
    return 0;
}
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o leak leak.c
$ ./leak
10 20 30
$ echo $?
0
```

Zero warnings. Correct output. Exit status 0. **Every signal available to
you says this program is fine, and it has a memory leak.** Now build the
identical source with the address sanitizer — a compiler feature that
instruments your program to watch every allocation and memory access:

```bash
gcc -std=c17 -Wall -Wextra -g -fsanitize=address,undefined -o leak_asan leak.c
./leak_asan
```

```
=================================================================
==61795==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 12 byte(s) in 1 object(s) allocated from:
    #0 0x7b4e9ed2c0c1 in malloc (/usr/lib/libasan.so.8+0x12c0c1)
    #1 0x6314c8a9e20d in main /tmp/ch10demo/leak.c:6

SUMMARY: AddressSanitizer: 12 byte(s) leaked in 1 allocation(s).
```

Twelve bytes, one object, allocated at `leak.c:6` — the exact line. Read
these reports from the top: the error type first, then the stack trace,
where the frame naming *your* file (`#1 ... in main leak.c:6`) is almost
always the one you care about. Frame `#0` is usually inside `malloc`
itself, which is true but unhelpful.

### Bug 2: use after free

This one is more dangerous, and demonstrating it honestly takes a bit of
care. Modern GCC catches the *obvious* version at compile time:

```
uaf.c: In function ‘main’:
uaf.c:15:5: warning: pointer ‘scores’ used after ‘free’ [-Wuse-after-free=]
   15 |     printf("scores[0] after free = %d\n", scores[0]);
      |     ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uaf.c:13:5: note: call to ‘free’ here
```

That's a genuinely useful warning — but real use-after-free bugs don't
look like that. They span function boundaries, where the compiler can't
see both halves at once. Split the allocation and release into their own
module (`scores_create`/`scores_destroy` in a separate `.c` file) and call
them from `main`:

```c
int *scores = scores_create();
if (scores == NULL) {
    fprintf(stderr, "out of memory\n");
    return 1;
}

scores_destroy(scores);

printf("scores[0] after destroy = %d\n", scores[0]);
```

```
$ gcc -std=c17 -Wall -Wextra -Wpedantic -g -o uaf2 uaf2.c scores.c
$ ./uaf2
scores[0] after destroy = -602197203
$ echo $?
0
```

**No warning. No crash. Exit status 0.** The program read memory it had
already given back and printed whatever was there. Run it again and you
may get a different number, or the same one, or something that looks
plausible. Now with the sanitizer:

```
=================================================================
==61834==ERROR: AddressSanitizer: heap-use-after-free on address 0x6ebc136e0010
READ of size 4 at 0x6ebc136e0010 thread T0
    #0 0x5cbe1125b2f1 in main /tmp/ch10demo/uaf2.c:14

0x6ebc136e0010 is located 0 bytes inside of 12-byte region [...]
freed by thread T0 here:
    #1 0x5cbe1125b2f0 in main /tmp/ch10demo/uaf2.c:12

previously allocated by thread T0 here:
    #1 0x5cbe1125b20b in main /tmp/ch10demo/uaf2.c:6
```

Three stack traces: where you *used* it, where you *freed* it, and where
you *allocated* it. That is enough to reconstruct the entire lifetime of
the bug without guessing.

Internalize the lesson underneath both bugs, because it's the most
valuable thing C teaches: **"it ran fine" proves nothing.** A C program
with undefined behaviour is not required to fail. It is free to work
perfectly on your machine, today, and fail on someone else's next month.
The sanitizer is how you find out which one you have.

### `gdb`, the debugger

The other tool worth meeting before you desperately need it. Compile with
`-g` (which this course has done since Chapter 0) and run under `gdb`:

```bash
gdb ./gdbdemo
```

Four commands cover most of what you'll want:

- `break scores_create` — pause when that function is entered
- `run` — start the program
- `bt` (backtrace) — show the call stack: who called whom to get here
- `print count` — show a variable's current value
- `next` — run one line, then pause again
- `continue` — resume until the next breakpoint

A real session, run with `gdb -batch` so the transcript is copyable:

```
Breakpoint 1, scores_create (count=3) at gdbdemo.c:6
6	    int *s = malloc((size_t)count * sizeof *s);
#0  scores_create (count=3) at gdbdemo.c:6
#1  0x00005555555551d7 in main () at gdbdemo.c:18
$1 = 3
7	    if (s == NULL) {
10	    for (int i = 0; i < count; i++) {
$2 = 0
10
[Inferior 1 (process 61983) exited normally]
```

Reading it: the breakpoint hit with `count=3`; `bt` shows `scores_create`
was called from `main` at line 18; `print count` gave `$1 = 3`; two
`next`s advanced through the `NULL` check into the loop; `print s[0]` gave
`$2 = 0` (the first element, already assigned). The `$1`/`$2` labels are
gdb numbering its answers so you can refer back to them.

ASan tells you *that* something is wrong and where. gdb lets you stop time
and look around. You'll want both.

## Apply it

Convert the map from a fixed global array into a heap-allocated structure
the game creates at startup and destroys at exit.

`map.h` — the map becomes a type with a documented owner:

```c
#ifndef MAP_H
#define MAP_H

typedef struct {
    int width;
    int height;
    char *tiles;
} Map;

/* Paired lifetime: every map_create must be matched by exactly one
   map_destroy. The caller owns the returned Map and is responsible for
   destroying it. Returns NULL if allocation fails. */
Map *map_create(void);
void map_destroy(Map *map);

char map_tile_at(const Map *map, int x, int y);
int map_is_walkable(const Map *map, int x, int y);

#endif
```

Note `char *tiles` — one flat block, not a 2D array. Chapter 5 showed that
a 2D array is already flat in memory; now you're allocating that flat
block directly, and doing the `y * width + x` arithmetic yourself.

`map.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

#define MAP_WIDTH  40
#define MAP_HEIGHT 20

static const char *const world_rows[MAP_HEIGHT] = {
    "########################################",
    /* ... the same 20 rows from Chapter 9 ... */
    "########################################"
};

Map *map_create(void)
{
    Map *map = malloc(sizeof *map);
    if (map == NULL) {
        return NULL;
    }

    map->width = MAP_WIDTH;
    map->height = MAP_HEIGHT;

    map->tiles = malloc((size_t)map->width * (size_t)map->height);
    if (map->tiles == NULL) {
        /* The Map itself was allocated but its tiles were not. Release
           the Map before giving up, or we leak it. */
        free(map);
        return NULL;
    }

    for (int y = 0; y < map->height; y++) {
        memcpy(map->tiles + (size_t)y * (size_t)map->width,
               world_rows[y],
               (size_t)map->width);
    }

    return map;
}

void map_destroy(Map *map)
{
    if (map == NULL) {
        return;
    }

    free(map->tiles);
    free(map);
}

char map_tile_at(const Map *map, int x, int y)
{
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return '#';
    }

    return map->tiles[(size_t)y * (size_t)map->width + (size_t)x];
}

int map_is_walkable(const Map *map, int x, int y)
{
    return map_tile_at(map, x, y) != '#';
}
```

Three things here are worth more than a passing glance:

**The partial-failure path in `map_create`.** Two allocations happen; the
second can fail after the first succeeded. If it does, `free(map)` before
returning `NULL` — otherwise the failure path leaks the very memory you're
about to lose the only pointer to. This is the most commonly skipped case
in real C code, and it is exactly the kind of thing ASan catches.

**`map_destroy` frees in the opposite order it allocated**, and frees
`map->tiles` *before* `map`. Freeing `map` first would leave `map->tiles`
unreachable — you'd need to read a member of a struct you just released,
which is use-after-free.

**`map_destroy(NULL)` is safe.** The early return means callers don't need
to check before calling. This mirrors `free` itself, which is defined to
accept `NULL` and do nothing — a convention worth adopting in your own
destroy functions.

`memcpy` (from `<string.h>`) copies raw bytes: destination, source, count.
It's the right tool here because you're copying exactly `width` bytes per
row and deliberately *not* copying the string terminator.

Since `MAP_WIDTH` is now private to `map.c`, `camera_center_on` can no
longer reach it — pass the bounds in instead:

```c
void camera_center_on(Camera *cam, int target_x, int target_y,
                      int map_width, int map_height);
```

```c
void camera_center_on(Camera *cam, int target_x, int target_y,
                      int map_width, int map_height)
{
    cam->x = clamp(target_x - VIEW_WIDTH / 2, 0, map_width - VIEW_WIDTH);
    cam->y = clamp(target_y - VIEW_HEIGHT / 2, 0, map_height - VIEW_HEIGHT);
}
```

`camera.c` no longer includes `map.h` at all — the camera now works with
any map dimensions handed to it, which is a genuine improvement that fell
out of the change rather than something you had to design for.

In `main.c`, create the map first and destroy it last, and pass it
everywhere the map is consulted:

```c
Map *map = map_create();
if (map == NULL) {
    fprintf(stderr, "Could not allocate the map. Out of memory?\n");
    return 1;
}
```

```c
if (!render_init()) {
    printf("Could not start the display. Is $TERM set correctly?\n");
    map_destroy(map);        /* don't leak on the error path either */
    return 1;
}
```

```c
camera_center_on(&camera, player.x, player.y, map->width, map->height);
draw_world(map, &player, &camera);
```

```c
render_shutdown();
duel_run();
map_destroy(map);

return 0;
```

That `map_destroy(map)` on the `render_init` failure path matters. Error
paths leak constantly in real code precisely because they're the paths
nobody exercises — and a leak on a path you take once before exiting is
harmless in practice, but the habit of thinking "what do I own here?" at
every `return` is not optional in C.

## Compile and run

```bash
make clean
make
./game
```

Zero warnings, and the game behaves *exactly* as it did in Chapter 9 — same
map, same scrolling, same collision. Nothing visible changed, which is the
point: this was an internal restructuring, and the way you know it worked
is that nothing broke.

Now the check that matters, and the one this course runs on every snapshot
from here on:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

Play for a bit, quit with `q`, finish the duel, and let the program exit
normally. **The correct result is complete silence from the sanitizer.**
Any output at all means a real bug. On a clean run you'll see the game's
own output and nothing else, with exit status 0.

Note that the sanitizer flags go in *both* `CFLAGS` and `LDLIBS` — ASan
needs to instrument each translation unit as it compiles *and* link its
runtime library into the final binary. Only doing the first produces
confusing link errors.

## What just happened

`map_create` made two requests to the heap: 16 bytes for the `Map` struct
(two `int`s and a pointer, plus padding), and 800 bytes for the tiles
(40 × 20). Both live outside any function's stack frame, so they survive
every call and return until `map_destroy` hands them back.

```
  main's stack frame                 HEAP
+--------------------+       +----------------------+
| map  ------------------->  | Map                  |
| player             |       |   width  = 40        |
| camera             |       |   height = 20        |
+--------------------+       |   tiles  ---------+  |
                             +-------------------|--+
                                                 v
                             +----------------------+
                             | 800 bytes:           |
                             | "####...####...####" |
                             |  (row 0, row 1, ...) |
                             +----------------------+

  map_destroy(map):  free(map->tiles)  then  free(map)
                     -- tiles first, or you can't reach it
```

`map->tiles[y * map->width + x]` is the same flat-memory arithmetic the
compiler was doing invisibly for `world[y][x]` back in Chapter 5. Writing
it by hand is the price of choosing the size at runtime.

## Common errors

**Freeing twice:**

```c
free(scores);
free(scores);
```

Back-to-back like this, GCC spots it at compile time (the same
`-Wuse-after-free` from earlier — passing a freed pointer to `free` counts
as using it):

```
dfree.c:9:5: warning: pointer ‘s’ used after ‘free’ [-Wuse-after-free=]
    9 |     free(s);
      |     ^~~~~~~
dfree.c:8:5: note: call to ‘free’ here
```

And ASan catches it at runtime, including the cross-function cases the
compiler can't see:

```
==62000==ERROR: AddressSanitizer: attempting double-free on 0x6f87476e0010 in thread T0:
    #0 0x73674952ae91  (/usr/lib/libasan.so.8+0x12ae91)
    #1 0x59bead53e250 in main /tmp/ch10demo/dfree.c:9
```

Without a sanitizer this often corrupts the allocator's internal
bookkeeping and crashes later, somewhere unrelated — a debugging nightmare.
The habit that prevents it: after `free(p)`, either the pointer goes out of
scope immediately, or you set `p = NULL` (freeing `NULL` is defined to do
nothing, so a double-free becomes harmless).

**Forgetting `<stdlib.h>`:**

```
map.c: In function ‘map_create’:
map.c:7:16: error: implicit declaration of function ‘malloc’ [-Wimplicit-function-declaration]
    7 |     Map *map = malloc(sizeof *map);
      |                ^~~~~~
map.c:2:1: note: include ‘<stdlib.h>’ or provide a declaration of ‘malloc’
    1 | #include <stdio.h>
  +++ |+#include <stdlib.h>
    2 |
```

Same shape as Chapter 1's missing `<stdio.h>`, and again a hard error on
modern GCC. `malloc`, `free`, and `NULL` all come from `<stdlib.h>`.

**Sanitizer flags in `CFLAGS` but not on the link line:**

```
/usr/bin/ld: main.o: in function `main':
main.c:(.text+0x2b): undefined reference to `__asan_report_load8'
collect2: error: ld returned 1 exit status
```

Chapter 4's `undefined reference` again, third variation: the ASan runtime
is a library, and libraries must appear when linking. Add
`-fsanitize=address,undefined` to `LDLIBS` as shown above.

## Exercises

1. Delete the `map_destroy(map)` call at the end of `main`, rebuild with
   sanitizers, and run. What does LeakSanitizer report, and how many bytes
   does it say leaked? Does it report one leak or two? (Think about
   whether losing the pointer to `map` also loses the pointer to
   `map->tiles`.) Put the call back afterward.
2. In `map_create`, delete the `free(map)` from the `tiles == NULL`
   failure branch. Does the program behave any differently in normal use?
   Why is this bug so much harder to notice than the one in exercise 1?
3. Add a `map_create_sized(int width, int height)` function that allocates
   a map of any requested size, filled entirely with floor tiles except
   for a wall border. Have `map_create` become a thin wrapper that calls
   it. (This is the change that makes runtime-sized maps genuinely useful,
   and it's a step toward Chapter 11, where maps come from files.)
4. *Open-ended:* The `Map` struct owns its `tiles` allocation. If you
   later add an `Npc` array to `Map` (Chapter 13 will), what would
   `map_destroy` need to do, and what would go wrong if two different
   `Map`s ever pointed at the *same* `tiles` block? Sketch the ownership
   rule you'd want written in the header comment.

<details>
<summary>Solutions</summary>

1. LeakSanitizer reports **two** leaks totaling 816 bytes:

   ```
   Direct leak of 16 byte(s) in 1 object(s) allocated from:
       #1 ... in map_create map.c:34
       #2 ... in main main.c:45

   Indirect leak of 800 byte(s) in 1 object(s) allocated from:
       #1 ... in map_create map.c:42
       #2 ... in main main.c:45

   SUMMARY: AddressSanitizer: 816 byte(s) leaked in 2 allocation(s).
   ```

   Two allocations, because `map_create` made two. Losing the pointer to
   `map` doesn't free `map->tiles`; it just makes it unreachable, which is
   precisely what a leak *is*. Note the classification: the 16-byte `Map`
   is a **direct** leak (nothing points at it any more), while the
   800-byte tile block is an **indirect** leak (something still points at
   it — but that something is itself leaked). When you see indirect leaks,
   fix the direct one first; the indirect ones usually disappear with it.

2. In normal use, nothing changes at all — that branch only runs if the
   second `malloc` fails, which essentially never happens for an 800-byte
   request on a modern machine. That's exactly what makes it dangerous:
   the bug is real, it's in shipped code, and no amount of ordinary
   testing will ever execute the line. Error paths need to be *reasoned*
   correct, not tested correct, because you usually can't trigger them on
   demand. (Tools exist to force allocation failures for exactly this
   reason.)

3. ```c
   Map *map_create_sized(int width, int height)
   {
       if (width <= 0 || height <= 0) {
           return NULL;
       }

       Map *map = malloc(sizeof *map);
       if (map == NULL) {
           return NULL;
       }

       map->width = width;
       map->height = height;
       map->tiles = malloc((size_t)width * (size_t)height);
       if (map->tiles == NULL) {
           free(map);
           return NULL;
       }

       for (int y = 0; y < height; y++) {
           for (int x = 0; x < width; x++) {
               int edge = (x == 0 || y == 0 ||
                           x == width - 1 || y == height - 1);
               map->tiles[(size_t)y * (size_t)width + (size_t)x] =
                   edge ? '#' : '.';
           }
       }

       return map;
   }
   ```
   The `width <= 0` check matters: `malloc(0)` is legal but returns
   something you must not dereference, and negative dimensions would
   produce a nonsensical (and possibly enormous, after conversion to
   `size_t`) allocation size.

4. `map_destroy` would need to free the NPC array before freeing `map`,
   same ordering rule as `tiles`. If two `Map`s shared one `tiles` block,
   destroying either would leave the other holding a dangling pointer, and
   destroying both would be a double-free — both undefined behaviour. The
   ownership rule to write down: *"A `Map` exclusively owns its `tiles`
   and `npcs` allocations. Do not copy a `Map` struct by assignment;
   copying the struct copies the pointers, not the memory, producing two
   owners for one allocation."* That last sentence is the real trap, and
   it's a direct consequence of Chapter 6's lesson that struct assignment
   copies every byte — including pointer members.

</details>

## Next up

The map data is still a set of string literals compiled into your program;
allocating it at runtime didn't change where it *comes from*. Chapter 11
introduces file I/O — `fopen`, `fgets`, parsing, and the error handling
that real input demands — so you can edit maps in a text editor and see
the change without recompiling.
