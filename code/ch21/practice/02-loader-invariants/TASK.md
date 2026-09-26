# 02 — Loader invariants (practice)

Read `LESSON.md` first.

## Build

```bash
make
make check
```

## Implement

In `main.c`, complete:

1. `is_sorted_by_level` — return 1 only when non-decreasing.
2. `effect_known` — strcmp against `KNOWN_EFFECTS`.
3. `validate_table` — on failure, `snprintf` into `reason` and return 0.
4. Observe the printed prefix vs scan note on the unsorted fixture.

## Done when

- `make check` exits 0.
- You can explain aloud why production keeps sortedness.

## Do not

- Remove the sortedness check from `code/ch21/magic.c`.
- Edit production spell assets for negative cases (use `fixtures/`).
