# 02 — Assertions vs tests (practice)

Read `LESSON.md` first.

## Build

```bash
make
make check
make demo-assert
# then run ./assertions_vs_tests_bad  (expect abort or FAIL)
```

## Implement

1. In `main.c`, add `assert(base >= 1);` after the floor (TODO1).
2. Keep the `#ifndef DEMO_BAD_FLOOR` floor in the default build.
3. Compare default `make check` vs the demo binary messages.

## Done when

- Default `make check` exits 0.
- You have run the demo build once and can describe both failure styles.

## Do not

- Delete the floor in production `combat_math.c`.
- Ship with `DEMO_BAD_FLOOR` defined.
