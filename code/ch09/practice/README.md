# Chapter 9 practice — camera math

Side-folder drills. These do **not** replace `camera_center_on` in the game.
The lasting camera is center-on-player + clamp. Variants live only here.

## Order

1. **`00-fundamentals/`** — clamp, center, bounds, world↔screen, visibility
   (short check-based programs; Bruce's micro-drills — left as-is this pass).
2. **`01-page-snap/`** — Zelda-style page camera with `/` and `%` (Monk).
3. **`02-dead-zone/`** — stateful dead-zone update (Monk).
4. **`03-clamp-and-center/`** — compare raw / page / clamped policies (Monk).
   Canonical live folder name (not the older out draft name
   `03-clamp-center-edge-cases`).

Also: `ANALOGIES_FOR_CHAPTER.md` (prose bank for the chapter).

Flags: `-std=c17 -Wall -Wextra -Wpedantic -g`.

```bash
# Fundamentals
(cd 00-fundamentals && make && ./01_clamp && ./02_center && ./03_bounds \
   && ./04_world_to_screen && ./05_visibility && make clean)

# Scenario drills (stubs may print until TODOs filled; see each SOLUTION.md)
(cd 01-page-snap && make clean && make && make check && make clean)
(cd 02-dead-zone && make clean && make && make check && make clean)
(cd 03-clamp-and-center && make clean && make && make check && make clean)
```

Do not paste solutions into `code/ch09/camera.c` to skip the reps.
