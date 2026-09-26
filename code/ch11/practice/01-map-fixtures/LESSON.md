# 01 — Map fixtures and parser diagnostics

Think of a map file as a **shipping manifest**: the game trusts width,
height, and every tile glyph the way a warehouse trusts crate labels. Hostile
inputs are mislabeled crates. The right lab is a shelf of deliberately bad
manifests (`fixtures/`), not vandalizing the only good manifest the warehouse
ships with (`assets/maps/overworld.map`).

Chapter 11's `map_load` must fail closed on bad headers, short rows, invalid
tiles, truncated grids, and absurd dimensions — and later accept `;` comment
lines without burning a row. By the end of this drill you should own those
checks as small pure helpers, name five malformed shapes from fixtures, and
reject the wrong reading that says "just break overworld.map and restore it."

## What this lesson asks of you

Read the fixture matrix and the worked examples. Reject corrupting production
assets for negative cases. Then open `TASK.md` and complete `parse_header`,
`row_ok`, and `count_data_lines` so `make check` passes.

## Foundations — fail closed at the boundary

A loader is a **boundary**. Inside the game you want a known-good `Map`. At
the boundary you owe clear rejection:

| Failure shape | Fixture | Typical check |
| ------------- | ------- | ------------- |
| Garbage header | `bad_header.map` | `sscanf` did not get two positive ints |
| Short row | `short_row.map` | trimmed length ≠ width |
| Illegal glyph | `invalid_tile.map` | char not in the tile alphabet |
| Too few rows | `truncated.map` | EOF before `height` data rows |
| Absurd size | `absurd_height.map` | dimension outside a sane cap |
| Comments | `comments.map` | `;` lines skip; row counter does not advance |

Happy path: `ok.map`. Comment sample: `comments.map`.

Two habits matter more than any single `sscanf` call:

1. **Name the failure** — stderr that says "short row" beats a silent NULL.
2. **Keep production assets pristine** — fixtures are disposable and
   comparable; a half-restored overworld looks like a camera bug next week.

## Worked example

**The situation.** You are rehearsing header parse before wiring `map_load`.

**Step 1 — accept a good header.**

```c
int w = 0, h = 0;
parse_header("40 20\n", &w, &h);  /* → 1, w==40, h==20 */
```

**Step 2 — reject garbage.**

```c
parse_header("nope\n", &w, &h);   /* → 0 */
parse_header("40\n", &w, &h);     /* → 0 — missing height */
```

**Step 3 — rejected wrong reading.** "Exercise 1 said corrupt
`overworld.map` five ways and restore it — that is how you learn fear of
input." The skill is right; the method is wrong. Easy to leave the game map
broken. Later chapters assume a known-good overworld. Fixtures named by
failure mode keep diagnostics comparable and the lasting tree untouched.
If you want to drive *real* `map_load`, copy a fixture to a throwaway path —
still never the shipped overworld.

## Distinctions worth keeping straight

- **Fixture vs production asset** — practice negatives on fixtures; ship
  only known-good maps under `assets/maps/`.
- **Comment line vs data row** — `;` skips without consuming a grid row.
  That is a lasting format feature when you add it to real `map_load`.
- **Row length vs tile alphabet** — short row and `X` in a wall/floor alphabet
  are different diagnostics; do not collapse them into one "bad line."

## Check yourself

1. Why does a half-restored `overworld.map` look like a logic bug in the
   camera or loader rather than "I forgot to undo an exercise"?
2. After `fgets`, when do you advance the row counter — every line, or only
   data lines?
3. Name five malformed-map shapes and which fixture file rehearses each.
4. Where should a lasting `;` comment feature live — only in this drill, or
   also in chapter `map_load` when the Exercises ask you to ship it?

## Key takeaways

- Hostile-input reps belong on disposable fixtures, not production assets.
- Fail closed at the loader boundary with clear diagnostics.
- `;` comment skipping is a format feature: skip without advancing rows.
- Practice helpers here; paste into `map_load` only when the chapter asks
  for the lasting comment feature.

## Lookup (not the lesson)

- Chapter 11 Apply it: `map_load` loop, `is_valid_tile`
- `sscanf` conversion count; trimming `\n`
- Chapter 21 later: validate-at-boundary for tables (same instinct)

Now open `TASK.md` and do the practice.
