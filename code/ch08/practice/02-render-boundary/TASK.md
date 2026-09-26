# 02 — Present at the boundary (practice)

Read `LESSON.md` first. This file is only the lab.

## Build (correct path)

```bash
make
./render_boundary
make check
```

## Build (leak — reading only)

```bash
make leak
./render_boundary_leak
```

Do **not** treat `game_leak.c` as a template for the chapter game. Do **not**
delete `render_present` from the playable tree. The lasting path keeps
`render_present`.

## What to verify

1. Correct `game_stub.c` includes only `render_stub.h` for drawing and calls
   `render_present()` at the end of `draw_world` — never `fake_refresh`.
2. `render_stub.c` is the only file that owns the backend flush on the
   default target.
3. `make check` runs the correct binary and exits 0 (two present calls).
4. Reading question (answer in your notes, not by editing the game):
   - After studying `game_leak.c` + the `leak` Makefile recipe, what
     `#include` would the real chapter's game logic need if `draw_world`
     called `refresh()` directly?
   - What lasting swap becomes harder once that include (or a raw backend
     call) lives in game logic?

## Done when

- `make` and `make check` succeed.
- You inspected `make leak` output / sources and can answer the reading
  questions aloud.
- The chapter game still has `render_present` declared and called.

## Do not

- Delete `render_present` (or its declaration) from the chapter game.
- Instruct yourself to "simplify" by inlining `refresh()` into `draw_world`
  on the lasting path.
- Add `#include <ncurses.h>` to game logic "for the exercise."
- Use the leak exhibit as anything other than a reading-only wrong path.
