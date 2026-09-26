# 03 — Key dispatch table (practice)

Read `LESSON.md` first. This file is only the lab.

## Build

```bash
make
./key_dispatch
make check
```

## Implement

In `main.c`, complete `input_from_key(int key)`:

1. Map `'w'`/`'W'` and `KEY_UP` → `INPUT_UP` (same for S/down, A/left, D/right).
2. Map `'q'`/`'Q'` → `INPUT_QUIT`.
3. Unknown keys → `INPUT_NONE`.
4. Keep the harness; do not change `InputEvent` or the fake `KEY_*` values.

## Done when

- `make` builds cleanly (`-std=c17 -Wall -Wextra -Wpedantic -g`).
- `make check` exits 0.
- You can answer aloud: where do raw keys become actions, and what does
  update consume?

## Do not

- Edit the chapter game's `input.c` / `main.c` for *this* drill's harness
  (arrow wiring in the real game is a separate lasting exercise).
- Put `player.y--` (or other world mutation) inside the key switch.
