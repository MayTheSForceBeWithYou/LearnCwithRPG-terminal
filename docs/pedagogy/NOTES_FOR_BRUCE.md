# Notes for Bruce — Ch08/Ch09 practice (live mirror)

Canonical practice trees live under `/workspace/learnc-rpg/live/code/ch0{8,9}/practice/`.
Out drafts under `/workspace/learnc-rpg/out/code/` are **reference prose** only —
do not blind-sync over live stubs (`00-fundamentals`, different `main.c` shapes,
`SOLUTION.md`).

## Live folder names (canonical)

| Path | Role |
| ---- | ---- |
| `live/code/ch08/practice/01-glyph-table/` | LESSON + TASK + stub + `make check` |
| `live/code/ch08/practice/02-render-boundary/` | LESSON + TASK + leak exhibit (`make leak`) |
| `live/code/ch08/practice/03-key-dispatch/` | LESSON + TASK + stub + `make check` |
| `live/code/ch09/practice/00-fundamentals/` | Bruce micro-programs (untouched this pass) |
| `live/code/ch09/practice/01-page-snap/` | scenario + `make check` |
| `live/code/ch09/practice/02-dead-zone/` | scenario + `make check` |
| `live/code/ch09/practice/03-clamp-and-center/` | **canonical name** (older out draft used `03-clamp-center-edge-cases`) |

Voice: adult/specific; **RPG** (avoid the J-prefixed genre tag). Side-folder drills; lasting path keeps
`render_present` and center+clamp camera.

## Builds

```bash
for d in 01-glyph-table 02-render-boundary 03-key-dispatch; do
  (cd live/code/ch08/practice/$d && make clean && make && make check)
done
for d in 01-page-snap 02-dead-zone 03-clamp-and-center; do
  (cd live/code/ch09/practice/$d && make clean && make && make check)
done
```

Flags on live practice: `-std=c17 -Wall -Wextra -Wpedantic -g`.

## Intent

- Ch08 Ex seam / glyph / key-dispatch → practice folders; do not delete
  `render_present` in the game.
- Ch09 page-snap / dead-zone / clamp compare → practice folders; do not paste
  over `camera_center_on` or delete `clamp` to silence warnings.
- `CameraMode` enum: keep side-only for now (agree).

## Style review

See `live/docs/pedagogy/STYLE_REVIEW_CH08_CH09.md` and
`out/docs/pedagogy/FIXES_APPLIED_CH08_CH09.md`.
