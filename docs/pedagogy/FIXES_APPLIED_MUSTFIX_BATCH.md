# Fixes applied — must-fix pedagogy batch (Ch11, Ch15, Ch21–23)

**Date:** 2026-09-25 PT  
**Authority:** `/workspace/learnc-rpg/out/docs/pedagogy/STYLE_REVIEW_MUSTFIX_BATCH.md`  
**Tree:** `/workspace/learnc-rpg/mustfix/`  
**Bar:** live Ch08/Ch09 (LESSON + TASK + `make check` + no production vandalism)

## Summary

Rewrote chapter Exercises that still instructed production vandalism; deepened
named scenario practice folders to otto-didact LESSON/TASK; added `make check`
across scenario + Bruce micro Makefiles; left Bruce's Ch15 `.c` / `solutions/`
content untouched aside from README index + Makefile `check`.

---

## A. Chapter Exercises rewritten

| File | Action |
| ---- | ------ |
| `/workspace/learnc-rpg/mustfix/chapters/ch11-reading-files.md` | Ex1 → fixtures/`01-map-fixtures`; forbid corrupt shipped overworld; lasting `;` comments stay Ex2 |
| `/workspace/learnc-rpg/mustfix/chapters/ch15-numbers-that-fight.md` | Light align: `make && make check`; explicit "do not delete production floor" |
| `/workspace/learnc-rpg/mustfix/chapters/ch21-data-driven-content.md` | Light align: `make check`; unsorted diagnosis stays in practice |
| `/workspace/learnc-rpg/mustfix/chapters/ch22-making-it-solid.md` | Ex1–2 → `01-tool-coverage` / `02-assertions-vs-tests` (`demo-assert`); no remove `memset` / delete floor |
| `/workspace/learnc-rpg/mustfix/chapters/ch23-the-ending.md` | Ex1 → `01-format-contract` + `make bad`; no delete production format attr |

---

## B. LESSON.md + TASK.md + slim README (scenario folders)

| Folder | Files |
| ------ | ----- |
| `/workspace/learnc-rpg/mustfix/code/ch11/practice/01-map-fixtures/` | **Created** LESSON.md, TASK.md; **slimmed** README; Makefile `check` |
| `/workspace/learnc-rpg/mustfix/code/ch21/practice/01-spell-line/` | **Created** LESSON.md, TASK.md; **slimmed** README; Makefile `check` |
| `/workspace/learnc-rpg/mustfix/code/ch21/practice/02-loader-invariants/` | **Created** LESSON.md, TASK.md; **slimmed** README; Makefile `check` |
| `/workspace/learnc-rpg/mustfix/code/ch22/practice/01-tool-coverage/` | **Created** LESSON.md, TASK.md; **slimmed** README; Makefile `check` |
| `/workspace/learnc-rpg/mustfix/code/ch22/practice/02-assertions-vs-tests/` | **Created** LESSON.md, TASK.md; **slimmed** README; Makefile `check` (+ kept `demo-assert`) |
| `/workspace/learnc-rpg/mustfix/code/ch23/practice/01-format-contract/` | **Created** LESSON.md, TASK.md; **slimmed** README; Makefile `check` (+ kept `bad`) |

Voice: advance organizer → foundations → worked example with rejected wrong reading → check yourself → takeaways → lookup.

---

## C. Index READMEs + Bruce micro `make check`

| File | Action |
| ---- | ------ |
| `/workspace/learnc-rpg/mustfix/code/ch11/practice/README.md` | `make && make check` |
| `/workspace/learnc-rpg/mustfix/code/ch15/practice/README.md` | Index note + `make check`; no wholesale rewrite of drills |
| `/workspace/learnc-rpg/mustfix/code/ch15/practice/Makefile` | **Added** `check:` |
| `/workspace/learnc-rpg/mustfix/code/ch21/practice/README.md` | Points at LESSON/TASK + `make check` |
| `/workspace/learnc-rpg/mustfix/code/ch22/practice/README.md` | Scenario order + micros + `make check` |
| `/workspace/learnc-rpg/mustfix/code/ch22/practice/Makefile` | **Added** `check:` |
| `/workspace/learnc-rpg/mustfix/code/ch23/practice/README.md` | Format-contract + `make bad` + micros |
| `/workspace/learnc-rpg/mustfix/code/ch23/practice/Makefile` | **Added** `check:` |
| `/workspace/learnc-rpg/mustfix/code/ch23/practice/02_phase_index.c` | **Minimal:** `#include <string.h>` so stub compiles (solution already had it) |

---

## D. Docs under out/

| File | Action |
| ---- | ------ |
| `/workspace/learnc-rpg/out/docs/pedagogy/STYLE_REVIEW_MUSTFIX_BATCH.md` | **Created** |
| `/workspace/learnc-rpg/out/docs/pedagogy/FIXES_APPLIED_MUSTFIX_BATCH.md` | **Created** (this file) |
| `/workspace/learnc-rpg/out/docs/pedagogy/HANDOFF_FOR_BRUCE.md` | **Created** |
| `/workspace/learnc-rpg/mustfix/docs/pedagogy/STYLE_REVIEW_MUSTFIX_BATCH.md` | **Copied** for WSL sync convenience |

---

## Verification (box, 2026-09-25 PT)

Flags: `-std=c17 -Wall -Wextra -Wpedantic -g`.

| Drill | Stub `make check` | Solutions / notes |
| ----- | ----------------- | ----------------- |
| Ch11 `01-map-fixtures` | fail (TODOs) — intentional | `solutions/*.c` exit 0 |
| Ch15 micros | fail (TODOs) — intentional | `solutions/*.c` exit 0 |
| Ch21 `01-spell-line` | fail — intentional | `solutions_main.c` exit 0 |
| Ch21 `02-loader-invariants` | fail — intentional | fill TODOs; no solutions `.c` yet (SOLUTION.md only) |
| Ch22 `01-tool-coverage` | exit 0 (observational) | keep `ready` unset |
| Ch22 `02-assertions-vs-tests` | exit 0 (floor on; assert TODO optional) | `make demo-assert` for bad floor |
| Ch22 micros | exit 0 | |
| Ch23 `01-format-contract` | exit 0 | `make bad`: without_exit=0, with_exit=1 |
| Ch23 micros | `02_phase_index` stub fails checks until TODO; compiles after string.h | `solutions/02_phase_index.c` OK |

Pass matrix one-liner: `scenario LESSON/TASK/check landed; stubs fail where TODO; ch11/ch15/ch21-spell solutions OK; format make bad OK; ch21-loader needs student/Bruce solution .c if desired.`

---

## Explicitly not done

- No wholesale rewrite of Bruce Ch15 practice `.c` / `solutions/`
- No game tree under mustfix (chapters + practice only in extract)
- No full-game `make valgrind` on this box (see HANDOFF)
- Ch21 `02-loader-invariants` still SOLUTION.md-only (no `solutions/*.c`) — Bruce may add
- Nice-to-haves N1–N5 from STYLE_REVIEW left open
