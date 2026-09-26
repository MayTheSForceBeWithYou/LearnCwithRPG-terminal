# 01 — Format contract (practice)

Read `LESSON.md` first.

## Build

```bash
make
make check
make bad
```

## Observe

1. `main.c` — good call with attribute; should run.
2. `bad_call.c` — intentional `%s`/int; used only by `make bad`.
3. Read `bad_without.txt` vs `bad_with.txt` (or the make echo). Confirm
   with-attr fails compile under `-Werror=format`.

No TODO required in `main.c` if it already carries the attribute — the lab
is the comparison.

## Done when

- `make check` exits 0.
- You have run `make bad` once and can quote the with-attr diagnostic.

## Do not

- Delete the format attribute from production `log_linef`.
- Link `bad_call.c` into the game.
