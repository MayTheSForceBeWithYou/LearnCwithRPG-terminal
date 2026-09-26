# Chapter 11 practice — reading map files

Side-folder drills. Do **not** corrupt `assets/maps/overworld.map` to
practice negative cases.

## Order

1. **01-map-fixtures/** — read `LESSON.md` then `TASK.md`; header / row /
   comment parsers + negative fixtures (replaces break-the-shipped-map as
   the main reps).

Also see fixtures inside that folder for `bad_header`, `short_row`,
`invalid_tile`, `truncated`, `absurd_height`, `comments`, and `ok`.

```bash
cd 01-map-fixtures && make && make check && make clean
```

Do not paste practice solutions into `code/ch11/map.c` except when the
chapter asks you to ship `;` comment skipping on the lasting path.
