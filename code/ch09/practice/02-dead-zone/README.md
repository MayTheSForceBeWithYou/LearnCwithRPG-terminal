# Drill 02: Dead-zone camera math

## Learning goal

Implement a **dead-zone** (or "look-ahead box") camera: the player can
wander inside a rectangle in the middle of the viewport without moving
the camera; the camera only scrolls when the player pushes against that
box's edge. This is the idea Chapter 9 exercise 4 asks you to *sketch* —
here you actually code the update rule in isolation.

## Why a dead zone feels different

Center-on-player: every step the camera recomputes from the player alone.
The player sits still on screen (until a clamp edge); the world slides.

Dead zone: the camera is **stateful**. This frame's camera depends on
*last frame's* camera plus whether the player left the box. The `@` can
drift inside the view. Crossing the box edge "pushes" the camera along.

## What you'll write

In `main.c`, complete `camera_update_dead_zone`:

- The dead zone is a rectangle in **screen** space, defined by margins
  (`MARGIN_X`, `MARGIN_Y`). Inside that box, do nothing to the camera.
- If the player's **screen** position would fall left of `MARGIN_X`,
  decrease `cam->x` so they sit on that edge (then clamp).
- Same idea for right, top, and bottom margins.
- You need the previous `cam->x` / `cam->y` — do not overwrite from the
  player alone the way `camera_center_on` does.

The harness walks a scripted path and prints camera + screen position
each step so you can see the dead stretch and the push.

## Instructions

```bash
make clean && make && ./dead_zone
```

Watch the "screen pos" column: it should drift within the margins while
you walk inside the zone, then stick to a margin when you push out.

## Hints

- Screen position given a camera: `screen_x = world_x - cam->x`
  (same conversion as Chapter 9).
- Push-left example: if `screen_x < MARGIN_X`, set
  `cam->x = world_x - MARGIN_X`, then clamp.
- Push-right: if `screen_x > VIEW_WIDTH - 1 - MARGIN_X`, set
  `cam->x = world_x - (VIEW_WIDTH - 1 - MARGIN_X)`, then clamp.
- Initialize the camera once (e.g. centered) before the walk loop; after
  that, only `camera_update_dead_zone` mutates it.

## Expected understanding

You can explain:

- why a dead-zone update must **read** the old camera, not only the player
- how screen-space margins turn into world-space camera adjustments
- why Chapter 9's open-ended sketch said "extra state between frames" —
  that state is literally `cam->x` / `cam->y` retained across updates
- that dead-zone is a design choice layered *on top of* clamping, not a
  replacement for it
