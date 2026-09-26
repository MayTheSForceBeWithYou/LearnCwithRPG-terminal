# 01 — Tool coverage (practice)

Read `LESSON.md` first.

## Build

```bash
make
make check
```

## Implement / observe

1. In `main.c`, leave `buggy_create`'s `ready` unset on purpose (TODO1).
2. Confirm `test_suite_path` never calls it; `covered_path` does.
3. Read the printed coverage lesson lines; answer the LESSON check-yourself
   questions aloud.

Optional: `valgrind --track-origins=yes ./tool_coverage` if available.

## Done when

- `make check` exits 0.
- You can explain quiet tests vs covered game path without editing
  production sources.

## Do not

- Delete `memset` from production `entity_create_player`.
- "Fix" `buggy_create` with memset — that erases the coverage exhibit.
