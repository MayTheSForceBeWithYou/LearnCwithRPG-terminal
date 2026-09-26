# Chapter 8 practice — game loop, glyphs, and the render seam

Side-folder drills. These do **not** replace `render_draw_tile`,
`render_present`, or anything under `code/ch08/*.c|h`. The lasting path
keeps the opaque render interface and a real `render_present` call.
Variants and "what if we deleted the seam?" experiments live only here.

## Order

1. **`01-glyph-table/`** — `TileType` → display-char table (Monk).
   Read `LESSON.md` then `TASK.md`. Change glyphs / add a tile kind without
   pasting into the live renderer.
2. **`02-render-boundary/`** — why `render_present` exists as a seam (Monk).
   Tiny two-file stub: game code calls `render_present`, not `refresh`.
   Optional `make leak` is a reading-only wrong-path exhibit. Do **not**
   delete `render_present` from the game or call `refresh()` from production
   `draw_world`.
3. **`03-key-dispatch/`** — keymap table / arrow-key dispatch (Monk).
   Chapter 8 exercise 2 (arrows in the live game) can stay as lasting UX;
   this drill owns the table-driven practice in isolation.

Also: `ANALOGIES_FOR_CHAPTER.md` (prose bank for the chapter).

Flags: `-std=c17 -Wall -Wextra -Wpedantic -g` (match chapter practice on WSL).

```bash
(cd 01-glyph-table && make clean && make && make check && make clean)
(cd 02-render-boundary && make clean && make && make check && make clean)
# optional reading exhibit:
(cd 02-render-boundary && make leak && ./render_boundary_leak && make clean)
(cd 03-key-dispatch && make clean && make && make check && make clean)
```

Do not paste solutions into `code/ch08/render_ncurses.c` or `input.c`
(or `main.c`) to skip the reps. Chapter 8 exercises 1–3 map onto these
drills as safer side practice.
