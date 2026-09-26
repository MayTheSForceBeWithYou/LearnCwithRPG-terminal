# 01 — Map fixtures (practice)

Read `LESSON.md` first. This file is only the lab.

## Build

```bash
make
make check
```

## Implement

1. In `01_header.c`, complete `parse_header` — two positive ints via `sscanf`;
   return 1 on success, 0 otherwise.
2. In `02_row_width.c`, complete `row_ok` — trim newline; length must equal
   `expect`; every char a valid tile (`#` or `.` in this drill).
3. In `03_skip_comments.c`, complete `count_data_lines` — skip lines whose
   first non-space character is `;`; count the rest (sample → 3).

Keep the harness `check(...)` calls. Compare `solutions/` only after a
failing check you cannot explain.

## Done when

- `make` builds cleanly (`-std=c17 -Wall -Wextra -Wpedantic -g`).
- `make check` exits 0 (runs all three binaries).
- You can name five fixture failure shapes without opening the chapter.

## Do not

- Edit or corrupt `assets/maps/overworld.map` for this drill.
- Paste practice solutions into `code/ch11/map.c` except when the chapter
  asks you to ship `;` comment skipping on the lasting path.
