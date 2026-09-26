# Handoff for Bruce — must-fix pedagogy batch

**Date:** 2026-09-25 PT  
**Tree to review:** `/workspace/learnc-rpg/mustfix/`  
**Pedagogy docs:** `/workspace/learnc-rpg/out/docs/pedagogy/STYLE_REVIEW_MUSTFIX_BATCH.md`,
`FIXES_APPLIED_MUSTFIX_BATCH.md` (this note).

Monk finished the pedagogy must-fixes for Ch11 / Ch15 / Ch21–23 against the
Ch08/Ch09 bar (LESSON + TASK + `make check` + no production vandalism).
Please do the **technical** pass on stubs, solutions, and tool exhibits.

## Paths (absolute)

### Chapters (Exercises rewritten)

- `/workspace/learnc-rpg/mustfix/chapters/ch11-reading-files.md`
- `/workspace/learnc-rpg/mustfix/chapters/ch15-numbers-that-fight.md`
- `/workspace/learnc-rpg/mustfix/chapters/ch21-data-driven-content.md`
- `/workspace/learnc-rpg/mustfix/chapters/ch22-making-it-solid.md`
- `/workspace/learnc-rpg/mustfix/chapters/ch23-the-ending.md`

### Scenario practice (LESSON / TASK / Makefile)

- `/workspace/learnc-rpg/mustfix/code/ch11/practice/01-map-fixtures/`
- `/workspace/learnc-rpg/mustfix/code/ch21/practice/01-spell-line/`
- `/workspace/learnc-rpg/mustfix/code/ch21/practice/02-loader-invariants/`
- `/workspace/learnc-rpg/mustfix/code/ch22/practice/01-tool-coverage/`
- `/workspace/learnc-rpg/mustfix/code/ch22/practice/02-assertions-vs-tests/`
- `/workspace/learnc-rpg/mustfix/code/ch23/practice/01-format-contract/`

### Bruce micros (content mostly yours; `check` targets added)

- `/workspace/learnc-rpg/mustfix/code/ch15/practice/` — **do not treat as rewritten**; index + `Makefile` only
- `/workspace/learnc-rpg/mustfix/code/ch22/practice/{01_assert_floor,02_ndebug_toy,03_fuzz_tokens}.c`
- `/workspace/learnc-rpg/mustfix/code/ch23/practice/{01_format_attr,02_phase_index,03_sim_potion_gate}.c`

## Intentional fail stubs

These **should** fail `make check` until the student fills TODOs:

| Path | Behaviour |
| ---- | --------- |
| `ch11/.../01_header.c` etc. | stubs return 0/−1; harness FAIL |
| `ch15/practice/01_*.c`–`04_*.c` | TODO stubs |
| `ch21/01-spell-line/main.c` | TODO parse |
| `ch21/02-loader-invariants/main.c` | TODO1–3; unsorted still prints NOTE lines |
| `ch23/02_phase_index.c` | TODO clamp; fails checks (compiles with `string.h`) |

Observational / already-green stubs (by design):

| Path | Note |
| ---- | ---- |
| `ch22/01-tool-coverage/main.c` | leave `ready` unset; harness passes; lesson is coverage text |
| `ch22/02-assertions-vs-tests/main.c` | floor on by default; add assert; use `make demo-assert` |
| `ch23/01-format-contract/main.c` | good call only; mismatch lives in `bad_call.c` |

Solutions present for Ch11 fixtures, Ch15, Ch21 spell-line, Ch22/Ch23 micros.
**Gap:** `02-loader-invariants` has `SOLUTION.md` but no `solutions/*.c` —
add if you want parity with Ch11.

## `make bad` / valgrind notes (Jack)

### Format contract (`make bad`)

```bash
cd /workspace/learnc-rpg/mustfix/code/ch23/practice/01-format-contract
make clean && make && make check && make bad
```

Expected on gcc 14 / `-Werror=format`:

- **Without** attribute (`-DNO_FORMAT_ATTR`): compile **succeeds** (`without_exit:0`);
  transcript often empty — custom wrappers are not format-checked without the attr.
- **With** attribute: compile **fails** (`with_exit:1`) on `%s` + `int` in `bad_call.c`.

Do **not** run the without-attr binary as a student requirement (UB). The
exhibit is the compile comparison.

### Assertions demo

```bash
cd /workspace/learnc-rpg/mustfix/code/ch22/practice/02-assertions-vs-tests
make && make check
make demo-assert && ./assertions_vs_tests_bad   # expect abort and/or FAIL
```

Floor drop is **only** via `-DDEMO_BAD_FLOOR`, never by editing production
`combat_math.c`.

### Valgrind / tool coverage

```bash
cd /workspace/learnc-rpg/mustfix/code/ch22/practice/01-tool-coverage
make && make check
# optional, if valgrind installed:
valgrind --track-origins=yes ./tool_coverage
```

Chapter no longer asks students to remove production `memset` to learn
coverage. Full-game `make valgrind` / ASan contrast remains a **WSL game-tree**
check (not in this mustfix extract). Please confirm on live WSL that
`run_tests` vs `./game` still teaches the same lesson the drill encodes.

## Minimal stub fix already applied

- `ch23/practice/02_phase_index.c`: added `#include <string.h>` (solution
  already had it). Stub still warns unused `lines` until TODO uses the table —
  acceptable, or silence if you prefer `-Werror` later.

## Freeze ask

Pedagogy for this batch is ready for your technical freeze review. Please
sign off stubs/solutions/`make bad`/optional valgrind, then sync
`mustfix/` → WSL live tree. Do not reintroduce chapter text that deletes
`memset`, damage floor, format attr, sortedness, or corrupts shipped
`overworld.map`.
