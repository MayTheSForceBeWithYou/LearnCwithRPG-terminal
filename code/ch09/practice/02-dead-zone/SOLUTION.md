# Solution — Drill 02: Dead-zone

```c
static void camera_update_dead_zone(Camera *cam, int world_x, int world_y)
{
    int screen_x = world_x - cam->x;
    int screen_y = world_y - cam->y;

    if (screen_x < MARGIN_X) {
        cam->x = world_x - MARGIN_X;
    } else if (screen_x > VIEW_WIDTH - 1 - MARGIN_X) {
        cam->x = world_x - (VIEW_WIDTH - 1 - MARGIN_X);
    }

    if (screen_y < MARGIN_Y) {
        cam->y = world_y - MARGIN_Y;
    } else if (screen_y > VIEW_HEIGHT - 1 - MARGIN_Y) {
        cam->y = world_y - (VIEW_HEIGHT - 1 - MARGIN_Y);
    }

    cam->x = clamp(cam->x, 0, MAP_WIDTH - VIEW_WIDTH);
    cam->y = clamp(cam->y, 0, MAP_HEIGHT - VIEW_HEIGHT);
}
```

## What to notice

- No branch fires while the player stays inside the margins — camera
  frozen, screen position drifts. That *is* the dead zone.
- The `Camera` struct already held the needed "extra state"; the function
  signature is what changed: update-in-place, not pure recompute.
- Clamp still runs every time. A dead zone that scrolls past the map is
  still a broken camera.
- Compared to `camera_center_on`, this is more code and more tuning
  (margin sizes). Many RPGs still prefer hard-center for overworld
  clarity; dead zones shine in denser action or when you want the player
  to "lead" into the scroll.

Still a side drill — don't replace `code/ch09/camera.c` unless a later
chapter deliberately adopts dead-zone as the game's camera.
