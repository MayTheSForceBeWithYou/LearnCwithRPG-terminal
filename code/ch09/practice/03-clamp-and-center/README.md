# Drill 03: Why clamp + center is the production path

## Learning goal

Compare three camera policies on the **same** walk script and map:

1. **Center, no clamp** — naive `target - view/2`
2. **Page-snap** — `(target / view) * view` (Drill 01)
3. **Center + clamp** — Chapter 9's lasting `camera_center_on`

See *when* each misbehaves (corners for raw center; jumpy screen placement for paging; overhang on non-multiple maps), and why the RPG keeps (3).

## The edge cases that matter

| Situation | Center only | Page-snap | Center+clamp |
|-----------|-------------|-----------|--------------|
| Player mid-map | player centered | player offset in page | player centered |
| Player near corner | camera goes **negative** / past map | jumps a whole page | pins to corner, `@` slides |
| Map not multiple of view | same overhang risk | last page overhangs unless clamped | always safe |
| Feel while walking | smooth follow | discrete room-screens | smooth follow |

"Goes negative" is the silent bug Chapter 9's Common errors section
describes: `draw_world` asks for world columns that don't exist. The map
accessor may paper over it with `'#'`, hiding an out-of-bounds intent.

## What you'll write

Complete `cam_center_raw`, `cam_page`, and `cam_center_clamped` in
`main.c`. The harness prints all three side-by-side for a walk that
visits a corner, the middle, and the far edge.

Also answer (in comments at the bottom of `main.c`, or aloud):

1. At player `(1, 1)`, what is raw center camera `x`? Why is that illegal?
2. At player `(38, 18)` on a 40×20 map with a 20×10 view, what does clamp
   set camera to, and where does the `@` appear on screen?
3. One sentence: why deleting `clamp` to silence a warning (after a
   page-snap rewrite) is the wrong trade.

## Instructions

```bash
make clean && make && ./clamp_center
```

Match the EXPECT columns. Read the verdict lines the program prints.

## Hints

- Raw center: `cam_x = target_x - VIEW_WIDTH / 2` (may be negative).
- Clamped center: wrap that in `clamp(..., 0, MAP_WIDTH - VIEW_WIDTH)`.
- Screen position of the player: `player - camera`. When clamped at a
  corner, this is **not** `VIEW_WIDTH / 2` — the `@` visibly slides, and
  that is correct.

## Expected understanding

You can defend Chapter 9's production camera in one paragraph: centering
for feel, clamping for safety, page-snap and dead-zone as *optional
alternatives* practiced beside the game rather than pasted over it.
