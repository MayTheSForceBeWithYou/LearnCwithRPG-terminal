# Solution — Drill 03: Clamp + center

```c
static void cam_center_raw(int tx, int ty, int *cx, int *cy)
{
    *cx = tx - VIEW_WIDTH / 2;
    *cy = ty - VIEW_HEIGHT / 2;
}

static void cam_page(int tx, int ty, int *cx, int *cy)
{
    *cx = (tx / VIEW_WIDTH) * VIEW_WIDTH;
    *cy = (ty / VIEW_HEIGHT) * VIEW_HEIGHT;
}

static void cam_center_clamped(int tx, int ty, int *cx, int *cy)
{
    *cx = clamp(tx - VIEW_WIDTH / 2, 0, MAP_WIDTH - VIEW_WIDTH);
    *cy = clamp(ty - VIEW_HEIGHT / 2, 0, MAP_HEIGHT - VIEW_HEIGHT);
}
```

## Written answers

1. At `(1, 1)`, raw `cam.x = 1 - 10 = -9`. Illegal: the viewport would
   include world columns `-9 .. 10`, and the first columns aren't on the
   map. (Whether `map_tile_at` paper-cuts that into `'#'` is irrelevant —
   the camera intent is still wrong.)

2. At `(38, 18)`, clamped cam is `(20, 10)` — the max legal origin.
   Screen pos is `(18, 8)`, not `(10, 5)`. The `@` has slid toward the
   bottom-right of the view; the world stopped scrolling. That slide *is*
   the clamp working.

3. Page-snap on an evenly divisible map can live without calling `clamp`,
   but deleting the helper to quiet `-Wunused-function` throws away the
   safety net the **production** camera still needs the moment you switch
   back — or the moment a map size stops dividing evenly. Prefer keeping
   `clamp` and practicing paging in a side file (this folder's siblings).

## Takeaway for Monk's chapter note

Exercise 3's skill (integer paging) is worth teaching; its *placement*
(overwrite `camera_center_on`, delete `clamp`) fights the path forward.
Drills 01–03 keep the skill and protect the lasting design.
