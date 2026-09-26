# Chapter 13: A World to Explore

## Where we are

You have one map, one NPC, and a text box. You also have — as of
Checkpoint B — a world worth building: the kingdom of Hollis, a crown
broken into four fragments, and an apologetic civil service that has hired
you to find them. This chapter turns one room into somewhere you can
actually travel: three connected areas, each with its own map file and its
own cast, linked by doors you can walk through.

## The problem

Everything about the current game is singular. One `Map *`. One `Npc`
declared as a local variable in `main`. Adding a second of anything means
adding a second variable, a second `if`, a second everything — and by the
sixth area you'd have an unreadable `main` and no way to add a seventh
without touching a dozen places.

What you want instead is for the world to be **data**: a table you can add
a row to. That's what arrays of structs and lookup tables are for, and
it's the single biggest step this course takes toward a real game's
architecture.

## C concept spotlight

### Arrays of structs

You've had arrays (Chapter 5) and structs (Chapter 6) separately. Together
they're how C represents "several of the same kind of thing":

```c
typedef struct {
    int x;
    int y;
    const char *speech;
} Npc;

Npc villagers[3] = {
    { .x = 8,  .y = 6,  .speech = "Bed's six gold." },
    { .x = 15, .y = 11, .speech = "Four pieces, scattered." },
    { .x = 26, .y = 16, .speech = "Eleven others declined." }
};

for (int i = 0; i < 3; i++) {
    printf("(%d,%d) %s\n", villagers[i].x, villagers[i].y,
           villagers[i].speech);
}
```

`villagers[i]` is a whole `Npc`; `villagers[i].x` reaches into it. In
memory this is exactly what Chapter 5's diagram showed — one contiguous
block, each element the full size of a struct, laid end to end:

```
villagers (3 x sizeof(Npc) bytes, contiguous):
+-----------------+-----------------+-----------------+
| x | y | speech* | x | y | speech* | x | y | speech* |
+-----------------+-----------------+-----------------+
  villagers[0]      villagers[1]      villagers[2]
```

Note that `speech` is a *pointer* into read-only string data, not the text
itself — the struct holds 8 bytes pointing elsewhere, not the sentence.
That's why copying an `Npc` is cheap, and also why the text it points at
must outlive the struct. String literals live for the whole program, so
this is safe; it would not be if `speech` pointed at a local buffer.

### The count problem

An array in C does not know how many elements are *in use*. `Npc npcs[4]`
is four `Npc`-sized slots whether you filled one or all four. So a struct
holding an array almost always holds a count beside it:

```c
typedef struct {
    Npc npcs[WORLD_MAX_NPCS_PER_AREA];
    int npc_count;
} Area;
```

Loop to `npc_count`, never to `WORLD_MAX_NPCS_PER_AREA` — the slots past
the count contain whatever the initializer left there, and treating them
as real NPCs is the same class of bug as reading past an array's end.

### Lookup tables: an enum as an array index

Here's the technique that makes the whole chapter work. Define an enum
whose values are consecutive from zero, and it can index an array:

```c
typedef enum {
    AREA_GRUBBIN_VALE,     /* 0 */
    AREA_WETWOOD_BARROW,   /* 1 */
    AREA_CASTLE_HOLLIS,    /* 2 */
    AREA_COUNT             /* 3 -- not an area, a count */
} AreaId;
```

The `AREA_COUNT` trick is worth internalizing: because enum values count
up automatically (Chapter 6), the last one equals the number of real
entries before it. Declare `Area areas[AREA_COUNT]` and the array is
always exactly the right size — add `AREA_MUDWICK` before `AREA_COUNT` and
the array grows by itself. No constant to keep in sync.

Then a **designated array initializer** makes the table's index explicit:

```c
static const Area area_table[AREA_COUNT] = {
    [AREA_GRUBBIN_VALE] = { .name = "Grubbin Vale", /* ... */ },
    [AREA_WETWOOD_BARROW] = { .name = "Wetwood Barrow", /* ... */ },
    [AREA_CASTLE_HOLLIS] = { .name = "Castle Hollis", /* ... */ }
};
```

`[AREA_GRUBBIN_VALE] =` says "this entry goes at that index" rather than
relying on the order you typed them in. Reorder the entries and it still
works; that's the point. Now `area_table[world->current]` is a complete
area lookup, in one array access, with no `switch` and no chain of `if`s.

### Linear search over a small table

Finding "the NPC at (x, y)" is a loop over the array:

```c
const Npc *world_npc_at(const World *world, int x, int y)
{
    const Area *area = world_current_area(world);

    for (int i = 0; i < area->npc_count; i++) {
        if (area->npcs[i].x == x && area->npcs[i].y == y) {
            return &area->npcs[i];
        }
    }

    return NULL;
}
```

Returning `&area->npcs[i]` — a pointer *into* the array — rather than a
copy means the caller sees the real thing, and `NULL` cleanly means "no
NPC there," exactly the convention Chapter 7 introduced. Returning
`const Npc *` promises the caller won't modify it through this pointer.

This is a linear search: it checks every element. For four NPCs that's
irrelevant; if you ever had four thousand you'd want something smarter.
Choosing the simple thing deliberately, and knowing why it's fine, beats
reaching for a hash table because it sounds more professional.


### Validate data before you trust coordinates

Lookup tables move bugs from "wrong code" into "wrong row in a table."
That is a win only if something checks the rows. A startup
`world_validate` that every NPC and warp sits on a walkable tile turns
mysterious in-game softlocks into a clear stderr line on launch. Practice
`03_validate_coords` is the miniature; exercise 2 asks for the real one —
keep it in the lasting tree.


## Apply it

### Three map files

Create `assets/maps/grubbin_vale.map`, `wetwood_barrow.map`, and
`castle_hollis.map`. They're different sizes on purpose — proof that the
runtime allocation from Chapter 10 and the loader from Chapter 11 were
worth building. Note the new `+` tiles: doors.

`grubbin_vale.map` (40×20), the starting village:

```
40 20
########################################
#......................................#
#..####...####...####..................#
#..####...####...####..................#
#..####...####...####...#####..........+
#.......................#####..........#
#.......................#####..........#
#..####...####..........#####..........#
#..####...####.........................#
#..####...####................######...#
#.............................######...#
#.............................######...#
#.............................######...#
#.....#####...######....####...........#
#.....#####...######....####...........#
#.....#####...######....####...........#
#......................................#
#......................................#
#......................................#
####################+###################
```

`wetwood_barrow.map` (24×12) and `castle_hollis.map` (36×16) follow the
same format; they're in `code/ch13/assets/maps/` if you'd rather copy them
than draw your own. Whatever you draw, **check that every area is actually
reachable** — a door you can't walk to is a bug you'll find much later and
blame on the wrong thing.

Teach the loader about doors, in `map.c`:

```c
static int is_valid_tile(char c)
{
    /* '#' wall, '.' floor, '+' door (walkable, and usually a warp). */
    return c == '#' || c == '.' || c == '+';
}
```

Because `map_is_walkable` already tests `!= '#'`, doors become walkable
with no further change. Add `TILE_DOOR` to `render.h`'s `TileType` and a
case in `render_ncurses.c`:

```c
case TILE_DOOR:   symbol = '+'; break;
```

### The world module

`world.h`:

```c
#ifndef WORLD_H
#define WORLD_H

#include "map.h"
#include "entity.h"

/* One entry per place the player can be. The order here must match the
   order of the table in world.c -- that is what makes an AreaId usable
   as an array index. */
typedef enum {
    AREA_GRUBBIN_VALE,
    AREA_WETWOOD_BARROW,
    AREA_CASTLE_HOLLIS,
    AREA_COUNT
} AreaId;

#define WORLD_MAX_NPCS_PER_AREA 4
#define WORLD_MAX_WARPS_PER_AREA 2

typedef struct {
    int x;
    int y;
    AreaId destination;
    int destination_x;
    int destination_y;
} Warp;

typedef struct {
    const char *name;
    const char *map_path;
    Npc npcs[WORLD_MAX_NPCS_PER_AREA];
    int npc_count;
    Warp warps[WORLD_MAX_WARPS_PER_AREA];
    int warp_count;
} Area;

typedef struct {
    Area areas[AREA_COUNT];
    Map *maps[AREA_COUNT];
    AreaId current;
} World;

/* Paired lifetime: every world_create must be matched by exactly one
   world_destroy. Loads every area's map up front; returns NULL (having
   freed anything it had already loaded) if any of them fail. */
World *world_create(void);
void world_destroy(World *world);

const Area *world_current_area(const World *world);
Map *world_current_map(const World *world);

/* Returns the NPC standing at (x, y) in the current area, or NULL. */
const Npc *world_npc_at(const World *world, int x, int y);

/* Returns the warp at (x, y) in the current area, or NULL. */
const Warp *world_warp_at(const World *world, int x, int y);

#endif
```

A `Warp` is pure data: where it is, and where it puts you. That's enough
to express every door in the game, and adding one is adding a row.

`world.c` — the table itself, which is now most of your game's content:

```c
static const Area area_table[AREA_COUNT] = {
    [AREA_GRUBBIN_VALE] = {
        .name = "Grubbin Vale",
        .map_path = "assets/maps/grubbin_vale.map",
        .npcs = {
            { .x = 8, .y = 6, .speech =
                "Bed's six gold. Sleep fixes most things, in my "
                "experience. Not everything. Most." },
            { .x = 15, .y = 11, .speech =
                "Four pieces. Scattered north, south, and -- the other "
                "two directions. I had it written down." },
            { .x = 26, .y = 16, .speech =
                "The Guild is prepared to offer forty gold up front and "
                "a completion bonus that I'd rather not describe as "
                "generous. Eleven others declined." }
        },
        .npc_count = 3,
        .warps = {
            { .x = 39, .y = 4, .destination = AREA_WETWOOD_BARROW,
              .destination_x = 1, .destination_y = 5 },
            { .x = 20, .y = 19, .destination = AREA_CASTLE_HOLLIS,
              .destination_x = 18, .destination_y = 1 }
        },
        .warp_count = 2
    },
    /* ... AREA_WETWOOD_BARROW and AREA_CASTLE_HOLLIS follow ... */
};
```

Those are Bess Harrow, Old Sudd, and Deputy Registrar Ammel, in the voice
Checkpoint B settled on. Castle Hollis holds two more — including a
nervous clerk with a line about a vault inventory. He is not important.
(He is extremely important. Chapter 23 explains.)

Creation and destruction, with the failure path handled:

```c
World *world_create(void)
{
    World *world = malloc(sizeof *world);
    if (world == NULL) {
        fprintf(stderr, "world_create: out of memory\n");
        return NULL;
    }

    for (int i = 0; i < AREA_COUNT; i++) {
        world->areas[i] = area_table[i];
        world->maps[i] = NULL;
    }

    for (int i = 0; i < AREA_COUNT; i++) {
        world->maps[i] = map_load(world->areas[i].map_path);
        if (world->maps[i] == NULL) {
            fprintf(stderr, "world_create: could not load area '%s'\n",
                    world->areas[i].name);
            /* Release the maps loaded before this one, then the World. */
            world_destroy(world);
            return NULL;
        }
    }

    world->current = AREA_GRUBBIN_VALE;
    return world;
}

void world_destroy(World *world)
{
    if (world == NULL) {
        return;
    }

    for (int i = 0; i < AREA_COUNT; i++) {
        map_destroy(world->maps[i]);
    }

    free(world);
}
```

The `world->maps[i] = NULL` loop before any loading is what makes the
failure path safe. If area 2 fails to load, `world_destroy` runs over
*all* three slots — and `map_destroy(NULL)` doing nothing (Chapter 10) is
exactly what makes that legal. Initialize pointers to `NULL` before
anything can fail, and cleanup becomes uniform instead of conditional.

`world->areas[i] = area_table[i]` copies the whole struct, including the
embedded arrays — Chapter 6's "assignment copies every byte," now doing
real work.

### Walking through doors

In `main.c`, movement grows two new checks: is there an NPC here (talk
instead of move), and did I land on a warp (change area)?

```c
if (dx != 0 || dy != 0) {
    int target_x = player.x + dx;
    int target_y = player.y + dy;

    const Npc *npc = world_npc_at(world, target_x, target_y);
    if (npc != NULL) {
        dialog_wrap(&dialog, npc->speech, TEXTBOX_WIDTH);
        mode = MODE_DIALOG;
    } else if (map_is_walkable(map, target_x, target_y)) {
        entity_move(&player, dx, dy);

        const Warp *warp = world_warp_at(world, player.x, player.y);
        if (warp != NULL) {
            world->current = warp->destination;
            player.x = warp->destination_x;
            player.y = warp->destination_y;
        }
    }
}
```

Three lines change the entire area. That's what the lookup table bought.

Drawing needs one guard that wasn't obvious until it broke:

```c
/* Only draw NPCs that actually fall inside the viewport. Without
   this check, an NPC standing off-screen is drawn at a screen
   coordinate outside the map area -- on top of the status line. */
for (int i = 0; i < area->npc_count; i++) {
    int sx = camera_world_to_screen_x(cam, area->npcs[i].x);
    int sy = camera_world_to_screen_y(cam, area->npcs[i].y);

    if (sx >= 0 && sx < VIEW_WIDTH && sy >= 0 && sy < VIEW_HEIGHT) {
        render_draw_tile(sx, sy, TILE_NPC);
    }
}
```

This is a genuine bug that appeared the first time the game ran with three
NPCs: one standing below the camera's view was drawn at screen row 10,
which is where the area name goes, producing a stray `o` next to "Grubbin
Vale." Chapter 9's coordinate-space discipline said convert world to
screen; it didn't say *check the result is on screen*. Now it does.

### The mode switch (deliberately awkward)

The game now has two modes, and handling them looks like this:

```c
switch (mode) {
    case MODE_EXPLORE: {
        int dx = 0;
        int dy = 0;

        switch (event) {
            case INPUT_UP:    dy = -1; break;
            /* ... */
        }

        if (dx != 0 || dy != 0) {
            /* ...movement, NPC check, warp check, all nested here... */
        }
        break;
    }

    case MODE_DIALOG:
        if (event != INPUT_NONE) {
            mode = MODE_EXPLORE;
        }
        break;
}
```

Read that and notice how it feels. A `switch` inside a `switch` inside a
loop, with movement logic three levels deep, and every future mode — a
menu, a shop, a battle — adding another `case` block to the same function.
It works. It is already unpleasant. **That is intentional**, and Chapter 14
is about fixing it properly rather than making it slightly tidier.

Add `world.c` to `SOURCES`.

## Compile and run

```bash
make clean
make
./game
```

You start in Grubbin Vale. Walking into Bess Harrow gets you a text box:

```
#...................
#..####...####...###
#..####...####...###
#..####...####...###
#...................
#......@o...........
#..####...####......
#..####...####......
#..####...####......
#...................
Grubbin Vale
 Bed's six gold.
 Sleep fixes most
 things, in my
 experience. Not
```

Head north to the open road, east to the `+` door on the right edge, and
step through:

```
####################
#...................
#.........####......
#...###...####......
#...###...####...###
+@..###.....o....###
#...###..........###
#...###...####......
#.........####......
#.........####......
Wetwood Barrow
w/a/s/d move, q quit
```

Different map, different size, different cast, and the area name at the
bottom changed. Step back onto the `+` behind you and you return to
Grubbin Vale, standing beside the door you left from.

And the sanitizer run:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

Three maps allocated at startup, all three freed at exit, silence from the
sanitizer.

## What just happened

`world_create` loaded three files into three heap allocations and copied
three table rows into the `World`. From then on, `world->current` — a
single enum value — decides which map is drawn, which NPCs are searched,
and which warps are checked, because all three are array lookups on that
one index.

```
World
+---------------------------------------------------+
| current = AREA_WETWOOD_BARROW  (1)                 |
|                                                    |
| areas[0] Grubbin Vale    npcs[3]  warps[2]         |
| areas[1] Wetwood Barrow  npcs[1]  warps[1]  <--- current
| areas[2] Castle Hollis   npcs[2]  warps[1]         |
|                                                    |
| maps[0] ---> Map 40x20 (heap)                      |
| maps[1] ---> Map 24x12 (heap)  <--- drawn          |
| maps[2] ---> Map 36x16 (heap)                      |
+---------------------------------------------------+

  Walking onto a warp is just:  current = warp->destination
```

Changing area is one assignment. No loading screen, no reallocation, no
special case — because every area was already loaded and every lookup was
already indexed by `current`. That's the payoff of tables over code.

## Common errors

**Looping to the array's capacity instead of its count:**

```c
for (int i = 0; i < WORLD_MAX_NPCS_PER_AREA; i++) {   /* should be npc_count */
    render_draw_tile(/* ... */ area->npcs[i].x, /* ... */);
}
```

No warning, no crash — Wetwood Barrow has one NPC in a four-slot array, so
this draws three phantom NPCs at `(0, 0)`, since the unused slots were
zero-initialized. If they *hadn't* been zero-initialized you'd be reading
uninitialized memory instead, which is undefined behaviour. Always loop to
the count.

**Forgetting to set `world->maps[i] = NULL` before loading:**

```c
World *world = malloc(sizeof *world);
/* straight into the load loop, no NULL initialization */
```

If area 1 fails to load, `world_destroy` calls `map_destroy` on
`maps[2]`, which was never assigned — `malloc` doesn't zero memory
(Chapter 10), so it holds garbage that gets dereferenced and freed:

```
map.c:122:13: runtime error: member access within misaligned address 0xbebebebebebebebe
              for type 'struct Map', which requires 8 byte alignment
0xbebebebebebebebe: note: pointer points here
<memory cannot be printed>
AddressSanitizer:DEADLYSIGNAL
```

That `0xbebebebe...` pattern is worth recognizing on sight: it's a filler
byte the sanitizer writes into uninitialized memory precisely so that
using it produces an obviously bogus address instead of something
plausible. If you ever see an address made of repeated `be`, `cd`, or
`ad` bytes, you are looking at uninitialized memory, not a real pointer.

**A warp whose destination isn't walkable:**

```c
{ .x = 39, .y = 4, .destination = AREA_WETWOOD_BARROW,
  .destination_x = 0, .destination_y = 0 }     /* (0,0) is a wall */
```

Nothing crashes and nothing warns — you simply arrive inside a wall and
can't move in any direction, because every neighbour is also wall. The
game is soft-locked and looks broken for no visible reason. Table data
gets no compile-time checking at all beyond its types, which is the
tradeoff for data-driven design and the motivation for exercise 2.

## Exercises

> **Practice drills:** `code/ch13/practice/` (`01_enum_index`–
> `04_count_slots`) before exercise 2.

1. *Practice.* Complete the practice folder (enum index, linear search,
   validate, count+slots).
2. *Durable — `world_validate`.* Implement startup validation: every NPC
   on a walkable tile; every warp on a door tile and destination walkable.
   Fail loudly. Keep this function — it is how `code/ch13/` stayed honest.
3. *Data, not code — optional area.* Add Mudwick (map file, `AREA_*`
   before `AREA_COUNT`, `area_table` row, warps both ways). Count how many
   *existing* functions you changed vs data you added. Prefer this after
   validate exists.
4. *Open-ended:* Forty areas — when load, when free, what about the
   `Map *` under the player's feet? Sketch ownership without implementing.

<details>
<summary>Solutions</summary>

1. See `code/ch13/practice/solutions/`.
2. Loop areas → NPCs / warps; `map_is_walkable`; fprintf + nonzero return.
3. Almost no function edits — enum + table + files. That ratio is the point.
4. Lazy load on enter; destroy previous when leaving; never free the active
   map until the replacement is loaded (or use a double-buffer swap).

</details>

## Next up

That nested `switch` in `main.c` is going to get worse the moment you add
a menu, and much worse when battles arrive. Chapter 14 introduces function
pointers and dispatch tables — the tool that turns "a `switch` with a case
per mode" into "a table of modes, each with its own handler" — and the
refactor will feel earned, because you've now felt the problem.
