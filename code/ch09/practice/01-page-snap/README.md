# Drill 01: Page-snap camera math

## Learning goal

Build a **page-scrolling** (snap) camera in isolation — the original
*Legend of Zelda* "one screenful at a time" feel — using integer `/` and
`%`. Leave the game's `camera_center_on` alone; this is a side folder so
you can learn the math without rewriting the production path.

## Why this is a drill (not Chapter 9 exercise 3)

Chapter 9's lasting camera is *center-on-player, then clamp*. Snapping to
pages is a useful *sibling* skill (and a great `/`/`%` workout), but if you
replace the game function with paging you:

- diverge from the camera every later chapter builds on
- can leave `clamp` unused (the course flags that as a warning)
- never practice the edge case that *does* need clamping: when the map
  size is **not** a multiple of the view size

Here you get both the happy path and the awkward last page.

## What you'll write

Complete the TODOs in `main.c`:

1. `page_origin(coord, view_size)` — snap a world coordinate down to the
   start of its page (`(coord / view_size) * view_size`).
2. `page_camera(target_x, target_y, ...)` — set camera `(x, y)` from the
   player's world position using page origins.
3. `page_camera_clamped(...)` — same idea, but clamp so a partial last
   page never scrolls past the map (try `MAP_WIDTH = 45` with
   `VIEW_WIDTH = 20`).

The program prints a table of player positions → camera origins. No
ncurses, no game link — just the numbers.

## Instructions

```bash
make clean && make && ./page_snap
```

Fill in each TODO until the "EXPECTED" column matches. Read the printed
notes about when clamping matters.

## Hints

- Integer division truncates toward zero for positive values:
  `27 / 20` is `1`, not `1.35`.
- `(n / v) * v` looks like it cancels; it doesn't — truncation snaps down
  to a multiple of `v`.
- Reconstruction check: `page * v + (n % v)` equals `n` for non-negative
  `n`.
- Upper clamp bound is still `MAP_WIDTH - VIEW_WIDTH` (and the height
  twin). If that bound goes negative (view larger than map), treat it as
  a design error for this drill — print a message rather than inventing
  a camera.

## Expected understanding

You can explain, without looking it up:

- why `/` then `*` snaps to a page boundary
- when page-snap needs **no** clamp (map size is a multiple of view size)
- when it **does** need a clamp (last page would overhang)
- why this belongs in a side drill, while the RPG keeps center+clamp
