# Chapter 9: The Camera

## Where we are

Chapter 8 gave you a real game loop: ncurses draws the screen, keys
register instantly, and the whole rendering layer hides behind
`render.h` so your game logic never mentions ncurses at all. But the world
is still a 10×5 room that fits comfortably on screen with room to spare.
Real JRPG worlds don't fit on screen — that's the entire reason walking
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
and `world_x % VIEW_WIDTH` says where you are within it. You'll build that
variant yourself in the exercises.

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

## Exercises

1. Change `VIEW_WIDTH` and `VIEW_HEIGHT` to `30` and `15`. Rebuild and
   walk to a corner. Does the clamping still behave correctly? What
   happens if you set `VIEW_WIDTH` to `40` (the full map width) — is the
   clamp's upper bound still meaningful?
2. Add a second `render_draw_text` call below the controls line that
   displays the player's current world coordinates, so you can watch them
   change as you walk. (You'll need `snprintf` to build the string —
   that's Chapter 12's topic, so peek ahead or just try
   `char buf[64]; snprintf(buf, sizeof buf, "pos: %d,%d", player->x,
   player->y);` and take it on faith for now.)
3. Implement the page-scrolling camera described in the spotlight section:
   instead of centering on the player, make the camera jump one full
   screenful at a time, so the view only changes when the player crosses a
   page boundary. Use `/` to compute the page. (Hint: `cam->x = (target_x
   / VIEW_WIDTH) * VIEW_WIDTH;` — and think about whether this version
   even needs clamping.)
4. *Open-ended:* This camera always centers on the player. Many JRPGs
   instead keep the player centered only until they approach a map edge,
   or use a "dead zone" — a box in the middle of the screen the player can
   move around freely inside before the camera starts following at all.
   Sketch how `camera_center_on` would need to change to support a dead
   zone. (What extra state would the camera need to remember between
   frames?)

<details>
<summary>Solutions</summary>

1. Clamping still works correctly at `30`×`15` — the bounds
   `MAP_WIDTH - VIEW_WIDTH` (10) and `MAP_HEIGHT - VIEW_HEIGHT` (5) just
   get smaller, so the camera has less room to scroll before pinning. At
   `VIEW_WIDTH = 40`, the upper bound becomes `40 - 40 = 0`, so
   `clamp(anything, 0, 0)` is always `0` — the camera can never scroll
   horizontally at all, which is exactly right, since the whole map width
   is already visible. The formula degrades gracefully rather than
   breaking, which is a good sign it's the right formula. (Setting
   `VIEW_WIDTH` *larger* than `MAP_WIDTH` would make the upper bound
   negative and `clamp(v, 0, -something)` would return the negative high
   value — a real bug, but one that only arises from a nonsensical
   configuration.)

2. ```c
   char buf[64];
   snprintf(buf, sizeof buf, "pos: %d,%d", player->x, player->y);
   render_draw_text(0, VIEW_HEIGHT + 2, buf);
   ```
   Note this goes on line `VIEW_HEIGHT + 2`, below the existing controls
   text at `VIEW_HEIGHT + 1`, so they don't overwrite each other.

3. ```c
   void camera_center_on(Camera *cam, int target_x, int target_y)
   {
       cam->x = (target_x / VIEW_WIDTH) * VIEW_WIDTH;
       cam->y = (target_y / VIEW_HEIGHT) * VIEW_HEIGHT;
   }
   ```
   The `/ VIEW_WIDTH` then `* VIEW_WIDTH` round-trip looks like it should
   cancel out, but integer division truncates — so it snaps `target_x`
   down to the nearest multiple of `VIEW_WIDTH`. This version needs no
   clamping at all: the largest page start is always within the map by
   construction, provided `MAP_WIDTH` is a multiple of `VIEW_WIDTH` (40
   and 20 — it is). If it weren't, the last page would run off the edge,
   and you'd want the clamp back. Notice how different this feels to play:
   the world only moves when you cross a boundary, which reads as more
   deliberate and less fluid.

   One practical wrinkle: dropping the clamp leaves `clamp` itself
   unused, and this course's flags treat that as worth mentioning —
   ```
   camera.c:4:12: warning: ‘clamp’ defined but not used [-Wunused-function]
   ```
   Delete the function (you can always retrieve it from `code/ch09/`) or
   keep the clamped version around under a different name. Leaving a
   warning in place isn't an option — that's the rule from Chapter 1, and
   it applies to your own experiments too.

4. No fixed answer. The key insight: a dead-zone camera can't compute its
   position from the player's position alone — it needs to remember where
   it was last frame, then only adjust if the player has moved outside the
   dead-zone box relative to the *current* camera position. So
   `camera_center_on` would stop being a pure "given the player, compute
   the camera" function and become an update-in-place one, reading
   `cam->x`/`cam->y` as input as well as writing them. The `Camera` struct
   already holds that state; what changes is that the function now depends
   on the camera's previous value instead of overwriting it
   unconditionally.

</details>

## Next up

Your map is a fixed-size array baked into the executable — every map you
ever add costs compile-time memory whether the player visits it or not,
and its size is locked at compile time. Chapter 10 introduces the heap:
`malloc`, `free`, ownership, and the debugging tools (`gdb`, and
sanitizers that catch memory bugs the compiler can't) that make manual
memory management survivable.
