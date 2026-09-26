# Chapter 22 practice — asserts, coverage, fuzzy inputs

Side-folder drills. Do **not** gut production `memset`, delete the damage
floor, or strip asserts from the game to rehearse these lessons.

## Order

1. **01-tool-coverage/** — `LESSON.md` / `TASK.md`; quiet tests vs covered path.
2. **02-assertions-vs-tests/** — assert vs CHECK; `make demo-assert` drops
   the floor in the **drill only**.
3. Micros (same directory): `01_assert_floor`, `02_ndebug_toy`,
   `03_fuzz_tokens`.

```bash
cd 01-tool-coverage && make && make check && make clean
cd ../02-assertions-vs-tests && make && make check && make clean
cd .. && make && make check && make clean
```
