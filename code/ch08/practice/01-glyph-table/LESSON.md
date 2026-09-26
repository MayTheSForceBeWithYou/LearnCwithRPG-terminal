# 01 — TileType → glyph table

Think of the renderer as a **two-column menu board**: game logic orders
"wall" or "floor"; the board decides which chalk mark appears. Changing wall
chalk from `#` to `%` is one board edit — you do not rewrite every order
ticket. Scattering `'#'` into `draw_world` is writing the chalk mark on every
ticket.

Chapter 8's `render_draw_tile` maps each `TileType` to a character before
calling the display library. Callers never pass `'#'` or `'@'` — they pass an
enum. By the end of this drill you should own that mapping as a small table,
change one glyph without touching any call site, and reject the wrong reading
that says "just put the character in `draw_world`."

## What this lesson asks of you

Read the table design and the worked example. Reject the wrong reading that
scatters glyphs through game logic. Then open `TASK.md` and complete
`glyph_for` in `main.c` so `make check` asserts floor ≠ wall ≠ player ≠ water
and that a wall glyph swap would be a one-cell table edit.

## Foundations — type in, glyph out

An opaque draw call looks like this:

```c
void render_draw_tile(int x, int y, TileType t);
```

Inside the render implementation (chapter: `render_ncurses.c`; here: this
drill's table), something must answer "what character stands for `t`?" Two
common shapes:

```c
/* Switch — chapter's first version */
switch (t) {
    case TILE_FLOOR:  symbol = '.'; break;
    case TILE_WALL:   symbol = '#'; break;
    case TILE_PLAYER: symbol = '@'; break;
}
```

```c
/* Table — same decision, data instead of control flow */
static const char GLYPH[] = {
    [TILE_FLOOR]  = '.',
    [TILE_WALL]   = '#',
    [TILE_PLAYER] = '@',
    [TILE_WATER]  = '~',
};
symbol = GLYPH[t];
```

Both keep the decision in **one place**. Call sites still say
`render_draw_tile(x, y, TILE_WALL)`. Changing walls from `#` to `%` edits one
initializer (or one `case`), not every place that ever drew a wall.

Designated initializers (`[TILE_WALL] = '#'`) index by enum value so the
order of lines does not have to match the enum declaration order — useful
when you add a tile type later and do not want a silent mis-map from a
positional array.

## Worked example

**The situation.** Four tile kinds (including water). A tiny "world" of five
cells is drawn by walking an array of `TileType` and looking up glyphs. You
want walls to become `%` for a dungeon experiment.

**Step 1 — call sites stay typed.**

```c
TileType row[] = { TILE_WALL, TILE_FLOOR, TILE_PLAYER, TILE_FLOOR, TILE_WALL };
for (int i = 0; i < 5; i++) {
    putchar(glyph_for(row[i]));   /* never putchar('#') here */
}
```

Printed with the default table: `#.@.#`

**Step 2 — change the table, not the loop.**

```c
[TILE_WALL] = '%',   /* was '#' */
```

Same loop, same `row[]`, new print: `%.@.%`

**Step 3 — rejected wrong reading.** "Faster to write `putchar('#')` when I
know it's a wall, and change those later if I care about looks." That couples
every call site to a visual choice. The chapter's whole point for
`render_draw_tile(int, int, TileType)` is that **appearance is an
implementation detail**. Scattering literals undoes the opaque interface
before ncurses even enters the story. If you ever swap terminals for a
graphical backend, those `'#'` literals in game logic are still wrong — they
were never the game's business.

## Distinctions worth keeping straight

- **Enum vs glyph** — `TILE_WALL` is meaning; `'#'` is presentation. Callers
  own meaning; the render module owns presentation.
- **Switch vs table** — same single-place rule; table makes "change data"
  literal. This drill uses a table so the harness can assert on the mapping.
- **Optional tweak in the game** — flipping `#` → `%` once inside
  `render_ncurses.c` is fine as a one-line curiosity. Doing it by editing
  `draw_world` is not. Prefer practicing the table here.
- **Asserts** — `glyph_for(TILE_WALL) != glyph_for(TILE_FLOOR)` catches
  accidental duplicate mappings that a quick eyeball miss.

## Check yourself

1. A caller has a wall at `(3, 2)`. What argument list should it pass to the
   draw function — characters, or a `TileType`?
2. You change only `GLYPH[TILE_WALL]`. Which files must recompile? Which
   source lines in `draw_world` must change?
3. Why is "delete the table and hard-code `'#'` in five loops" the wrong
   lasting habit even if the screenshot looks identical today?
4. Where should a lasting one-line glyph experiment live if you try it in the
   chapter game — `main.c` or `render_ncurses.c`?

## Key takeaways

- `TileType` → `char` is a single-module decision; call sites pass the enum.
- A table (or switch) lets you change wall appearance without hunting call
  sites.
- Opacity starts before ncurses: even a printf harness should not leak glyphs
  into game-shaped loops.
- Practice glyph swaps here; keep the game's `render_draw_tile` signature and
  call sites on the typed path.

## Lookup (not the lesson)

- Chapter 8 spotlight: opaque interfaces; `render_draw_tile` signature
- C99 designated initializers for arrays
- Chapter 6 enums (where `TileType` sits in the larger type story)

Now open `TASK.md` and do the practice.
