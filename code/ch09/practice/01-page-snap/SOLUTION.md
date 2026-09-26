# Solution — Drill 01: Page-snap

```c
static int page_origin(int coord, int view_size)
{
    return (coord / view_size) * view_size;
}

static void page_camera(int target_x, int target_y, int *cam_x, int *cam_y)
{
    *cam_x = page_origin(target_x, VIEW_WIDTH);
    *cam_y = page_origin(target_y, VIEW_HEIGHT);
}

static void page_camera_clamped(int target_x, int target_y,
                                int map_w, int map_h,
                                int *cam_x, int *cam_y)
{
    int x = page_origin(target_x, VIEW_WIDTH);
    int y = page_origin(target_y, VIEW_HEIGHT);
    *cam_x = clamp(x, 0, map_w - VIEW_WIDTH);
    *cam_y = clamp(y, 0, map_h - VIEW_HEIGHT);
}
```

## What to notice

- On the even map, `page_camera` and `page_camera_clamped` agree for every
  sample — clamp never fires.
- On the odd map, player `(44, 22)` wants page `(40, 20)`, but the max
  legal camera is `(25, 13)`. Clamp pulls it back so the viewport's far
  edge lands on the map's far edge.
- Chapter 9 exercise 3's "you don't need clamp" claim is true *only* when
  map dims are multiples of view dims. The production `camera_center_on`
  keeps clamp because centering near edges has the same overhang problem
  even on "even" maps.

Do **not** paste this into `code/ch09/camera.c`. The RPG keeps
center-and-clamp; paging lives here as a studied alternative.
