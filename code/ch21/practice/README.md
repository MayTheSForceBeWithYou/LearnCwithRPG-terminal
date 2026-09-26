# Chapter 21 practice — data-driven loaders

Side-folder drills. Do **not** remove the sortedness check from production
`magic_load_spells` / `magic.c`.

## Order

1. **01-spell-line/** — `LESSON.md` / `TASK.md`; parse one spell row (names
   with spaces).

2. **02-loader-invariants/** — validate-at-boundary with malformed spell
   fixtures; see why `known_count` assumes sorted levels.

```bash
cd 01-spell-line && make && make check && make clean
cd ../02-loader-invariants && make && make check && make clean
```

Ship content edits belong in `assets/`; invariant experiments belong here.
