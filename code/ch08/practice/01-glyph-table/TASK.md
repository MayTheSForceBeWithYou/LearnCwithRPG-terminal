# 01 — TileType → glyph table (practice)

Read `LESSON.md` first. This file is only the lab.

## Build

```bash
make
./glyph_table
make check
```

## Implement

In `main.c`, complete `glyph_for(TileType t)`:

1. Return the character from a `static const char` table indexed by `t`
   (designated initializers). Defaults: floor `'.'`, wall `'#'`, player
   `'@'`, water `'~'`. Unknown / out-of-range may return `'?'`.
2. Keep the harness checks; do not remove them. After a one-cell wall edit
   to `'%'`, call sites of `glyph_for` must not need to change (try it as an
   experiment, then restore `'#'` so `make check` still passes).

## Done when

- `make` builds cleanly (`-std=c17 -Wall -Wextra -Wpedantic -g`).
- `make check` exits 0.
- You can answer aloud: where does a wall glyph live, and what do call sites
  pass instead?

## Do not

- Edit the chapter game's `render_ncurses.c` or `main.c` for this drill.
- Put `'#'` / `'%'` / `'~'` literals into game-shaped draw loops.
