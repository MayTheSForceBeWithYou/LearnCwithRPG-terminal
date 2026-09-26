# Chapter 9: The Camera

## Where we are

Chapter 8 gave you a real game loop: ncurses draws the screen, keys
register instantly, and the whole rendering layer hides behind
`render.h` so your game logic never mentions ncurses at all. But the world
is still a 10×5 room that fits comfortably on screen with room to spare.
Real RPG worlds don't fit on screen — that's the entire reason walking
around them is interesting. This chapter makes the world four times bigger
than the visible area and introduces the thing that decides what you can
see: the camera.

## The problem

Once the world is bigger than the screen, a coordinate stops having one
obvious meaning. Is `x = 24` the 24th column of the *world*, or the 24th
column of your *terminal*? Those are different numbers that both look like
plain `int`s, and mixing them up produces bugs that are genuinely hard to
see — the map draws, the player draws, nothing crashes, and everything is
just subtly in the wrong place. You need two clearly separated coordinate
spaces and a disciplined way to convert between them.

## C concept spotlight

### Coordinate spaces

A **coordinate space** is just an agreement about what a pair of numbers
means and where its origin sits. This chapter has two:

- **World space** — position in the map data. `(0, 0)` is the map's
  top-left corner; `x` runs to `MAP_WIDTH - 1` (39), `y` to
  `MAP_HEIGHT - 1` (19). The player's position lives here.
- **Screen space** — position on the visible display. `(0, 0)` is the
  top-left character cell of the viewport; `x` runs to `VIEW_WIDTH - 1`
  (19), `y` to `VIEW_HEIGHT - 1` (9). `render_draw_tile` expects these.

The camera is what connects them. It holds one world-space position — the
world coordinate currently shown at screen `(0, 0)` — and that single
offset defines the entire conversion:

```
screen_x = world_x - camera.x
world_x  = screen_x + camera.x
```

```
World (40 x 20), camera at (20, 0):

  0                   20                  39
  +-------------------+===================+
0 |                   ‖ VIEWPORT          ‖
  |                   ‖ (what you see)    ‖
  |    not drawn      ‖   20 x 10         ‖
9 |                   ‖                   ‖
  |                   +===================+
  |                                       |
19+---------------------------------------+

  A tile at world (24, 3) draws at screen (24 - 20, 3 - 0) = (4, 3).
```

Nothing enforces this distinction for you — both are `int`, and C will
cheerfully let you pass a world coordinate where a screen coordinate
belongs. The discipline has to come from naming (`world_x` vs `screen_x`,
never bare `x`) and from routing every conversion through one place.

**Two rulers, one desk.** Imagine two rulers taped to the same desk. One
measures from the map's top-left (world). The other measures from the
viewport's top-left (screen). Both can mark "12," and both are wrong for
the other job. The camera is the sticky note that says which world column
lines up with screen column 0. Mix the rulers and nothing crashes — the
`@` just quietly sits in the wrong place, which is worse.


### Clamping

If the camera simply centered on the player unconditionally, standing near
a map edge would scroll the view *past* the edge, showing rows and columns
that don't exist — reading `world[y][x]` out of bounds, which Chapter 5
established is undefined behaviour. **Clamping** constrains a value to a
range:

```c
static int clamp(int value, int low, int high)
{
    if (value < low)  return low;
    if (value > high) return high;
    return value;
}
```

```
clamp(-4, 0, 20)  = 0
clamp(7, 0, 20)   = 7
clamp(33, 0, 20)  = 20
```

Three lines, no cleverness — and it's the entire reason the camera can
follow the player without ever exposing off-map space. The upper bound
deserves a moment's thought: the largest valid camera `x` is
`MAP_WIDTH - VIEW_WIDTH` (40 - 20 = 20), because at that position the
viewport's right edge sits exactly on the map's right edge. One more, and
you'd be reading column 40 of a 40-column map — the classic off-by-one
from Chapter 5, wearing a different hat.

### Integer division and modulo, together

You've used `/` on integers since Chapter 2, where it truncated `15 / 20`
to `0` and cost you a percentage. Its companion `%` (the **modulo**
operator) gives the part that division threw away — the remainder:

```c
for (int world_x = 0; world_x < 45; world_x += 9) {
    int page   = world_x / VIEW_WIDTH;
    int offset = world_x % VIEW_WIDTH;
    printf("world_x %2d -> page %d, offset %2d  (page*%d + offset = %d)\n",
           world_x, page, offset, VIEW_WIDTH, page * VIEW_WIDTH + offset);
}
```

```
world_x  0 -> page 0, offset  0  (page*20 + offset = 0)
world_x  9 -> page 0, offset  9  (page*20 + offset = 9)
world_x 18 -> page 0, offset 18  (page*20 + offset = 18)
world_x 27 -> page 1, offset  7  (page*20 + offset = 27)
world_x 36 -> page 1, offset 16  (page*20 + offset = 36)
```

`/` and `%` are two halves of one operation: `page * VIEW_WIDTH + offset`
always reconstructs the original number exactly. This pairing is how a
*page-scrolling* camera works — the original *Legend of Zelda*'s
screen-at-a-time rooms, rather than the smooth following this chapter
builds. `world_x / VIEW_WIDTH` says which screenful you're standing in,
and `world_x % VIEW_WIDTH` says where you are within it.

That page-snap formula is **alternate math** worth learning; it is not this
chapter's shipped camera. The game keeps clamped centering. Explore page-snap
in the side drills under `code/ch09/practice/01-page-snap/` (see Exercises).

One caution for later, since it bites people: with *negative* numbers, C's
`%` follows the sign of the left operand, so `-7 % 20` is `-7`, not `13`.
Verified on this toolchain:

```
-7 / 20 = 0
-7 % 20 = -7
```

If you ever apply `%` to coordinates that might go negative, that's a real
trap — for this chapter, clamping keeps everything non-negative, so it
stays theoretical.


### Visibility (what later chapters will need)

Centering and conversion answer "where is the camera?" and "where does this
tile draw?" A third question appears the moment anything other than the
player exists in the world: **is this world position on screen at all?**

```c
int camera_is_on_screen(const Camera *cam, int world_x, int world_y)
{
    return world_x >= cam->x && world_x < cam->x + VIEW_WIDTH
        && world_y >= cam->y && world_y < cam->y + VIEW_HEIGHT;
}
```

The interval is half-open: inclusive on the left, exclusive on the right —
the same convention as C array indexing and as the draw loop's
`screen_x < VIEW_WIDTH`. A point on the right edge column
`cam->x + VIEW_WIDTH - 1` is visible; `cam->x + VIEW_WIDTH` is not.

Why this belongs in the camera module rather than in `draw_world`: every
future caller that places an NPC, a warp marker, or a projectile will need
the same test. Putting it behind `camera.h` means Chapter 13 does not
reinvent the inequality — and means a bug in the inequality is fixed once.

Work this until it is boring: drills `01`–`05` in `code/ch09/practice/`
exist specifically so the inequalities become muscle memory before you
wire them into the game.

### Edge cases worth naming out loud

**Odd view sizes.** `VIEW_WIDTH / 2` truncates. With `VIEW_WIDTH == 21`,
a player at world `x` produces camera `x - 10`, so the player sits one
cell left of true center. That is not a bug — integer pixels (and
character cells) cannot split in half — but if you ever see an off-by-one
"the player isn't centered," check whether the view size is odd before
rewriting the formula.

**`VIEW` equal to `MAP`.** Upper clamp bound becomes `0`. The camera
never scrolls. Correct, and a good sanity check that the formula
degrades cleanly.

**`VIEW` larger than `MAP`.** Upper bound goes negative.
`clamp(v, 0, -5)` returns `-5` with the three-line clamp above — a
camera origin outside the map. Do not paper over this inside `clamp`.
Treat it as a configuration error (see practice drill `03_bounds`). The
shipped game keeps `VIEW_*` smaller than `MAP_*` on purpose.

**Drawing only the player without a visibility check.** Today the player
is always on screen *by construction* of `camera_center_on` (clamping
guarantees the target stays inside the view). The moment a second entity
exists at an arbitrary world coordinate, that guarantee vanishes — hence
`camera_is_on_screen` before `render_draw_tile` for anything that is not
the camera's follow target.

## Apply it

### A bigger world

Replace `map.c`'s 10×5 grid with a 40×20 one. Since hand-editing 20 rows
of comma-separated character literals would be miserable, switch to string
literals — one row per line, directly editable:

```c
/* map.h */
#define MAP_WIDTH  40
#define MAP_HEIGHT 20

int map_is_walkable(int x, int y);
char map_tile_at(int x, int y);
```

```c
/* map.c */
/* Each row is written as a string literal, so the map stays editable by
   hand. A string literal carries one extra byte at the end (the NUL
   terminator -- Chapter 12 covers it properly), which is why the row
   arrays are MAP_WIDTH + 1 bytes wide. That last byte is never drawn. */
static const char world[MAP_HEIGHT][MAP_WIDTH + 1] = {
    "########################################",
    "#......#################...............#",
    "#......#################...#########...#",
    "#......#####........####...#.......#...#",
    "#............#######.......#.......#...#",
    "#......#####........####...#...#####...#",
    "#......#################...#...........#",
    "#......#################...#############",
    "#..........................#############",
    "############...#############..........##",
    "############...#############..........##",
    "#..............#############..........##",
    "#...####################...............#",
    "#...####################...#########...#",
    "#...................####...#########...#",
    "#########..............................#",
    "#########...####################...#####",
    "#.......................########...#####",
    "#.......................########.......#",
    "########################################"
};
```

The `MAP_WIDTH + 1` is the fix for the warning Chapter 5 hit head-on:
a 40-character string literal needs 41 bytes of storage because of its
hidden terminating byte. Declaring the rows one byte wider gives that byte
somewhere legitimate to live, so the string-literal form compiles cleanly.
`const` is new here too — the map is fixed data that no code should
modify, and marking it `const` gets the compiler to enforce that, the same
promise `const Player *` made in Chapter 7.

Add a tile accessor alongside the walkability check, since more than one
caller now wants to know what's actually at a coordinate:

```c
char map_tile_at(int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return '#';
    }

    return world[y][x];
}

int map_is_walkable(int x, int y)
{
    return map_tile_at(x, y) != '#';
}
```

Out-of-bounds coordinates report `'#'` — treating everything outside the
map as solid wall. That's a deliberate design choice, not just defensive
padding: it means "off the edge of the world" and "into a wall" are the
same thing to every caller, so no caller needs a separate bounds check.

### The camera module

`camera.h`:

```c
#ifndef CAMERA_H
#define CAMERA_H

#define VIEW_WIDTH  20
#define VIEW_HEIGHT 10

typedef struct {
    int x;
    int y;
} Camera;

void camera_center_on(Camera *cam, int target_x, int target_y);
int camera_world_to_screen_x(const Camera *cam, int world_x);
int camera_world_to_screen_y(const Camera *cam, int world_y);

#endif
```

`camera.c`:

```c
#include "camera.h"
#include "map.h"

static int clamp(int value, int low, int high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

void camera_center_on(Camera *cam, int target_x, int target_y)
{
    cam->x = clamp(target_x - VIEW_WIDTH / 2, 0, MAP_WIDTH - VIEW_WIDTH);
    cam->y = clamp(target_y - VIEW_HEIGHT / 2, 0, MAP_HEIGHT - VIEW_HEIGHT);
}

int camera_world_to_screen_x(const Camera *cam, int world_x)
{
    return world_x - cam->x;
}

int camera_world_to_screen_y(const Camera *cam, int world_y)
{
    return world_y - cam->y;
}
```

`camera_center_on` is the whole algorithm in two lines: put the target in
the middle (`target_x - VIEW_WIDTH / 2`), then clamp so the view never
leaves the map. `camera_center_on` takes a `Camera *` because it modifies
the camera; the two conversion functions take `const Camera *` because they
only read it — the Chapter 7 discipline, applied consistently.

### Drawing through the camera

In `main.c`, `draw_world` now iterates over *screen* space and converts to
*world* space to decide what to draw:

```c
static void draw_world(const Player *player, const Camera *cam)
{
    render_clear();

    for (int screen_y = 0; screen_y < VIEW_HEIGHT; screen_y++) {
        for (int screen_x = 0; screen_x < VIEW_WIDTH; screen_x++) {
            int world_x = cam->x + screen_x;
            int world_y = cam->y + screen_y;

            TileType t = map_is_walkable(world_x, world_y)
                       ? TILE_FLOOR
                       : TILE_WALL;

            render_draw_tile(screen_x, screen_y, t);
        }
    }

    render_draw_tile(camera_world_to_screen_x(cam, player->x),
                     camera_world_to_screen_y(cam, player->y),
                     TILE_PLAYER);

    render_draw_text(0, VIEW_HEIGHT + 1,
                     "w/a/s/d to move, q to quit wandering");

    render_present();
}
```

Every variable says which space it's in. The loop bounds are screen-space
(`VIEW_WIDTH`/`VIEW_HEIGHT`) because the loop's job is "fill every visible
cell"; the map lookup is world-space because that's what `map_is_walkable`
speaks. `render_draw_tile` always receives screen coordinates, without
exception — including for the player, which is exactly what those two
conversion functions are for.

The game loop just recomputes the camera before each draw:

```c
Camera camera;
InputEvent event = INPUT_NONE;

while (event != INPUT_QUIT) {
    camera_center_on(&camera, player.x, player.y);
    draw_world(&player, &camera);

    event = input_poll();
    /* ... movement handling, unchanged from Chapter 8 ... */
}
```

Finally, add `camera.c` to `SOURCES` in the `Makefile`:

```make
SOURCES = main.c entity.c map.c camera.c duel.c render_ncurses.c input.c
```

## Compile and run

```bash
make clean
make
./game
```

Zero warnings. Walk around and watch the difference from Chapter 8: near
the top-left corner the view is pinned and only the `@` moves, but once
you get toward the middle of the map, the *world* slides under a
stationary player. Starting at the top-left, you should see the camera
clamped at `(0, 0)`:

```
####################
#@.....#############
#......#############
#......#####........
#............#######
#......#####........
#......#############
#......#############
#...................
############...#####
```

After walking south 7 and east 23 (down the left corridor, then along the
long open row), the camera has scrolled in both directions and the player
sits near the center of the view:

```
......####...#......
######.......#......
......####...#...###
##########...#######
##########...#######
..........@..#######
.#############......
.#############......
.#############......
##########..........
```

And at the far right edge of the map, the camera clamps again — the player
keeps moving toward the edge while the view stops:

```
####################
####..............@#
####...#########...#
####...#.......#...#
.......#.......#...#
####...#...#####...#
####...#...........#
####...#############
.......#############
########..........##
```

## What just happened

Every frame does the same three steps: decide where the camera should be
(`camera_center_on`, clamped), then for each visible cell convert screen →
world and ask the map what's there, then draw at the screen coordinate.
The player never actually moves on screen in the middle of the map —
`player.x` grows, `camera.x` grows by exactly the same amount, and
`player.x - camera.x` stays put at `VIEW_WIDTH / 2`. Near the edges the
clamp freezes `camera.x`, so the subtraction starts changing again and the
`@` visibly moves instead. That's the whole illusion: one subtraction,
plus a clamp that decides when the world scrolls versus when the player
does.


The inequalities in `camera_is_on_screen` are the same half-open range the
draw loop already uses; once NPCs arrive you will call it before every
non-player `render_draw_tile`. Page-scrolling and dead-zone cameras are
real designs — they live in `code/ch09/practice/` as drills so you can
learn them without demolishing the following camera this codebase keeps
from here forward.

## Common errors

**Drawing world coordinates directly to the screen:**

```c
render_draw_tile(player->x, player->y, TILE_PLAYER);   /* missing conversion */
```

No warning, no crash — and the player appears in the wrong place the
moment the camera scrolls off `(0, 0)`, drifting further from its true
position the further you walk. Worse, once `player->x` exceeds
`VIEW_WIDTH`, ncurses is being asked to draw outside the viewport
entirely, and the `@` simply vanishes. This is the coordinate-space bug in
its purest form, and the reason every variable in `draw_world` is named
`world_*` or `screen_*` rather than plain `x`/`y`.

**Forgetting to clamp the camera:**

```c
cam->x = target_x - VIEW_WIDTH / 2;   /* no clamp */
```

Standing at world `x = 1` puts `cam->x` at `-9`, so `draw_world`'s first
column asks for `map_is_walkable(-9, ...)`. Because `map_tile_at` bounds-
checks and returns `'#'`, this happens to render as harmless wall rather
than reading out of bounds — the map module's defensive check absorbing a
bug in a completely different module. Had `map_tile_at` indexed `world`
directly without checking, this would instead be an out-of-bounds read:
undefined behaviour, quite possibly no crash, and a much longer debugging
session. This is what defensive bounds checks are *for* — not the error
you predicted, but the one you didn't.

**Off-by-one on the clamp's upper bound:**

```c
cam->x = clamp(target_x - VIEW_WIDTH / 2, 0, MAP_WIDTH);   /* should be MAP_WIDTH - VIEW_WIDTH */
```

At the right edge, the viewport now extends 20 columns past the map, and
the right half of the screen fills with the `'#'` that `map_tile_at`
returns for out-of-bounds coordinates — a solid wall that isn't in your
map data anywhere. If you ever see a mysterious wall exactly at the map
boundary, check this bound first.


> **Practice drills:** `code/ch09/practice/` — start with `00-fundamentals/`, then Monk's `01-page-snap` / `02-dead-zone` / `03-clamp-and-center`. See that folder's `README.md`.

## Exercises

Complete the practice work **before** exercise 3. Drills live under
`code/ch09/practice/` — standalone programs, no ncurses, no link against
the game. Clamping and coordinate conversion need reps, not a single paste
into `camera.c`.

1. *Reading check.* With `MAP_WIDTH = 40`, `VIEW_WIDTH = 20`, and the
   player at world `(1, 1)`, what is `cam->x` after `camera_center_on`?
   What is the player's *screen* x? Now put the player at `(38, 1)` and
   answer the same two questions. Paper first; confirm with
   `00-fundamentals/02_center` or by printing in a throwaway build.

2. *Practice folder (required).* Work through, in order:
   - `code/ch09/practice/00-fundamentals/` (drills `01_clamp`–
     `05_visibility` — each must print `all checks passed`)
   - `01-page-snap/` (`make && make check`) — page math; **do not** paste
     into the game camera
   - `02-dead-zone/` — stateful update
   - `03-clamp-and-center/` — why production keeps center+clamp
   Do not open `solutions/` / `SOLUTION.md` until you have a failing check
   you cannot explain.

3. *Durable integration — visibility on the real path.* Add
   `camera_is_on_screen(const Camera *cam, int world_x, int world_y)` to
   `camera.h` / `camera.c` (half-open bounds; see the Visibility spotlight
   and `00-fundamentals/05_visibility`). Guard the player draw in
   `draw_world` with it. Leave the API in place for Chapter 13's NPCs.
   Do **not** replace `camera_center_on` with a page-scroller — that is
   `01-page-snap`, not the shipped game. (Monk and this chapter agree:
   side-folder variants; lasting path stays clamped follow.)

4. *Open-ended design.* After `02-dead-zone`, sketch how you would expose
   both follow and dead-zone modes later without `#ifdef` soup. A mode
   enum defaulting to follow is fine as a *future* product idea — it is
   not required for this chapter (see `docs/pedagogy/NOTES_FOR_BRUCE.md`).

<details>
<summary>Solutions</summary>

1. At world `(1, 1)`: raw center x is `1 - 10 = -9` → clamp → `cam->x = 0`.
   Player screen x is `1 - 0 = 1`. At world `(38, 1)`: raw center
   `38 - 10 = 28` → clamp to `MAP_WIDTH - VIEW_WIDTH = 20` → `cam->x = 20`.
   Player screen x is `38 - 20 = 18`. Near the right edge the `@` sits close
   to the right of the viewport instead of the center — that is the clamp
   doing its job, not a centering bug.

2. Each drill's harness is the specification. Fundamentals solutions:
   `00-fundamentals/solutions/`. Scenario solutions: each folder's
   `SOLUTION.md`. Honest attempt first.

3. ```c
   int camera_is_on_screen(const Camera *cam, int world_x, int world_y)
   {
       return world_x >= cam->x && world_x < cam->x + VIEW_WIDTH
           && world_y >= cam->y && world_y < cam->y + VIEW_HEIGHT;
   }
   ```
   In `draw_world`:
   ```c
   if (camera_is_on_screen(cam, player->x, player->y)) {
       render_draw_tile(camera_world_to_screen_x(cam, player->x),
                        camera_world_to_screen_y(cam, player->y),
                        TILE_PLAYER);
   }
   ```
   With the following camera this `if` is always true for the player; the
   point is the API shape Chapter 13 reuses. Snapshot: `code/ch09/`.

4. No fixed answer. Dead-zone needs previous `cam->x`/`cam->y`. Prefer an
   explicit call site or a small mode enum *later*; keep practice harnesses
   as the oracle for non-default modes so the game binary is not the only
   place the math is tested.

</details>

## Next up

Your map is a fixed-size array baked into the executable — every map you
ever add costs compile-time memory whether the player visits it or not,
and its size is locked at compile time. Chapter 10 introduces the heap:
`malloc`, `free`, ownership, and the debugging tools (`gdb`, and
sanitizers that catch memory bugs the compiler can't) that make manual
memory management survivable.
