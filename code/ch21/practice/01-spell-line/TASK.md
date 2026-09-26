# 01 — Spell line (practice)

Read `LESSON.md` first.

## Build

```bash
make
make check
```

## Implement

In `main.c`, complete `parse_spell_line`:

1. Parse `level`, `mp`, `mag`, `effect`, and a name that may contain spaces.
2. Return 1 on success, 0 on malformation.
3. Keep harness checks (including `"Second Wind"`).

## Done when

- `make check` exits 0.
- You can explain the name-tail scanset aloud.

## Do not

- Edit `code/ch21` game sources or `assets/spells.txt` for this drill.
- Remove production sortedness checks (that is not this folder).
