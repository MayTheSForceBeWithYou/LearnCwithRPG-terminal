# Style / pedagogy review — must-fix batch (Ch11, Ch15, Ch21–23)

**Reviewer role:** box-only review + must-fix apply for Monk before Bruce freeze.  
**Date:** 2026-09-25 PDT  
**Primary tree:** `/workspace/learnc-rpg/mustfix/` (WSL extract)  
**Bar:** `/workspace/learnc-rpg/live/` Ch08/Ch09 (LESSON.md + TASK.md + `make check` + no production vandalism)  
**Also consulted:** `/workspace/learnc-rpg/out/docs/pedagogy/STYLE_REVIEW_CH08_CH09.md`, `/workspace/learnc-rpg/mustfix/docs/pedagogy/AUDIT_WEAK_EXERCISES.md`

**Preferences applied:** adult/specific tone; **RPG** (avoid the J-prefixed genre tag); otto-didact LESSON depth (thin README = incompleteness); side-folder drills; chapter Exercises must not instruct deleting production invariants (`render_present`, clamp, `memset`, damage floor, format attr, sortedness, corrupt shipped `overworld.map`).

---

## 0. Inventory (absolute box paths)

### Chapters

| Path | Role |
| ---- | ---- |
| `/workspace/learnc-rpg/mustfix/chapters/ch11-reading-files.md` | Exercises still told students to corrupt shipped overworld |
| `/workspace/learnc-rpg/mustfix/chapters/ch15-numbers-that-fight.md` | Exercises already retargeted to practice (Bruce shape) |
| `/workspace/learnc-rpg/mustfix/chapters/ch21-data-driven-content.md` | Blockquote already forbids removing sortedness |
| `/workspace/learnc-rpg/mustfix/chapters/ch22-making-it-solid.md` | Exercises still remove `memset` / delete floor in production |
| `/workspace/learnc-rpg/mustfix/chapters/ch23-the-ending.md` | Exercise 1 still deletes production format attribute |

### Practice (scenario folders named in brief)

| Path | Pre-fix state |
| ---- | -------------- |
| `/workspace/learnc-rpg/mustfix/code/ch11/practice/01-map-fixtures/` | Dense README (~1.5 KB); **no** LESSON/TASK; **no** `make check` |
| `/workspace/learnc-rpg/mustfix/code/ch21/practice/01-spell-line/` | Thin README (~250 B); no LESSON/TASK; no `make check` |
| `/workspace/learnc-rpg/mustfix/code/ch21/practice/02-loader-invariants/` | Usable README; no LESSON/TASK; no `make check` |
| `/workspace/learnc-rpg/mustfix/code/ch22/practice/01-tool-coverage/` | Usable README; no LESSON/TASK; no `make check` |
| `/workspace/learnc-rpg/mustfix/code/ch22/practice/02-assertions-vs-tests/` | Usable README + `demo-assert`; no LESSON/TASK; no `make check` |
| `/workspace/learnc-rpg/mustfix/code/ch23/practice/01-format-contract/` | Usable README + `make bad`; no LESSON/TASK; no `make check` |

### Bruce micro-drills (integrate-only / leave content)

| Path | Note |
| ---- | ---- |
| `/workspace/learnc-rpg/mustfix/code/ch15/practice/` | `01_clamp_damage`–`04_xorshift_seed` + `solutions/` — **do not rewrite wholesale** |
| `/workspace/learnc-rpg/mustfix/code/ch22/practice/{01_assert_floor,02_ndebug_toy,03_fuzz_tokens}.c` | Flat micros beside scenario folders |
| `/workspace/learnc-rpg/mustfix/code/ch23/practice/{01_format_attr,02_phase_index,03_sim_potion_gate}.c` | Flat micros beside `01-format-contract` |

**Genre-tag:** no hits under `/workspace/learnc-rpg/mustfix/` at review time.

---

## 1. What's good

### Architecture / lasting path

- Side-folder doctrine is **already started** for this batch: every chapter has a `code/chNN/practice/` tree, and Ch11/Ch15/Ch21 chapter Exercises already carry blockquotes that *name* practice and forbid some vandalism.
- Fixture doctrine for hostile maps (Ch11) and loader invariants (Ch21) matches the audit theme bank — disposable files named by failure mode.
- Ch22 `02-assertions-vs-tests` already has `-DDEMO_BAD_FLOOR` / `make demo-assert` — correct exhibit pattern (wrong path without editing production).
- Ch23 `01-format-contract` already has `make bad` comparing with/without `__attribute__((format(...)))` on a disposable `bad_call.c`.
- Ch15 chapter Exercises are already in good shape: practice first, durable monotonicity on the harness, flee *wiring* not rewrite, open-ended spell damage sketch.

### Writing / teaching tone (chapters)

- Adult/specific throughout these mid/late chapters (hostile inputs, tool coverage vs program coverage, product decision on `NDEBUG`, balance sims).
- Reference games / systems talk without a J-prefixed genre label in this extract.

### Completeness vs chapter Exercises (mapping — post-fix target)

| Chapter exercise | Practice home | Pre-fix gap |
| ---------------- | ------------- | ------------ |
| Ch11 hostile inputs | `01-map-fixtures` | Ex1 still corrupted shipped map |
| Ch11 `;` comments | practice + lasting `map_load` | OK direction |
| Ch15 math/RNG | Bruce `01`–`04` | Integrate-only |
| Ch21 spell line / invariants | `01-spell-line`, `02-loader-invariants` | Thin prose / no `make check` |
| Ch22 tool coverage | `01-tool-coverage` | Chapter still removed production `memset` |
| Ch22 assert vs test | `02-assertions-vs-tests` | Chapter still deleted production floor |
| Ch23 format attr | `01-format-contract` | Chapter still deleted production attribute |
| Ch23 items/phases/sim | lasting game path | Keep (ship features) |

---

## 2. Must-fix gaps (punch-list)

### A. Chapter Exercises that still instruct production vandalism

| # | Edit |
| - | ---- |
| A1 | **Ch11:** Rewrite Ex1 so negative cases use `01-map-fixtures/fixtures/`; forbid corrupting shipped `assets/maps/overworld.map`. Keep lasting `;` comment skip as Ex2. |
| A2 | **Ch22:** Replace Ex1–2 production "remove memset / delete floor" with pointers to `01-tool-coverage` and `02-assertions-vs-tests` (`make demo-assert`). Keep conceptual NDEBUG + open-ended fuzzer as reading/product, not game gutting. |
| A3 | **Ch23:** Replace Ex1 "delete format attribute on `log_linef`" with `01-format-contract` + `make bad`. Keep items-in-battle / phase / sim as lasting or open-ended. |
| A4 | **Ch15 / Ch21:** Light align only — ensure blockquotes + Solutions point at practice; do not wholesale rewrite Ch15 body. Ch21 already forbids removing sortedness — keep and ensure Ex text matches. |

### B. Thin README → LESSON.md + TASK.md (otto-didact)

Required sections: advance organizer ("what this lesson asks"), foundations, worked example with **rejected wrong reading**, check yourself (2–4 Qs), key takeaways, optional lookup. Slim README to a one-liner pointer.

| # | Folder |
| - | ------ |
| B1 | `code/ch11/practice/01-map-fixtures/` |
| B2 | `code/ch21/practice/01-spell-line/` |
| B3 | `code/ch21/practice/02-loader-invariants/` |
| B4 | `code/ch22/practice/01-tool-coverage/` |
| B5 | `code/ch22/practice/02-assertions-vs-tests/` |
| B6 | `code/ch23/practice/01-format-contract/` |

### C. `make check` where drills / chapter promise completion

| # | Edit |
| - | ---- |
| C1 | Add `check:` to every scenario Makefile above (run binary; fail on nonzero). |
| C2 | Add `check:` to Bruce micro Makefiles (`ch15`, `ch22` top-level, `ch23` top-level) without rewriting `.c` content. |
| C3 | Update practice index READMEs to `make && make check`. |

### D. Guardrails

| # | Edit |
| - | ---- |
| D1 | Grep Solutions / LESSON / chapter Exercises: "delete memset/floor/format/sortedness/corrupt overworld" must remain **warnings against**, not student steps. |
| D2 | Do **not** clobber Bruce's `ch15/practice/*.c` or `solutions/` beyond README/`Makefile` index touch. |

---

## 3. Nice-to-have

| # | Idea |
| - | ---- |
| N1 | Per-micro one-paragraph organizers under Ch15 / Ch22 / Ch23 flat drills (Bruce may prefer leave thin). |
| N2 | Ch11 optional "drive real `map_load` from a *copy* of a fixture" callout — already hinted; keep optional. |
| N3 | Ch22 `01-tool-coverage`: optional `make valgrind` target if `valgrind` present (document as optional; Jack notes). |
| N4 | Ch23 flat `02_phase_index.c` appears to use `strcmp` without `#include <string.h>` — Bruce tech fix, not this pedagogy pass. |
| N5 | Fold ANALOGIES banks if later added for these chapters (none in mustfix extract). |

---

## 4. Freeze readiness for Bruce

### Verdict: **Ready for Bruce technical review after must-fixes in this pass land**

| Area | Freeze-ready? | Why |
| ---- | ------------- | --- |
| Chapter Exercises (this batch) | **Yes after A1–A4** | Side-drill doctrine + no production vandalism |
| Scenario practice LESSON/TASK/`make check` | **Yes after B+C** | Meets Ch08/Ch09 bar for named folders |
| Bruce Ch15 micros | **Yes (integrate-only)** | Content untouched; index/`check` only |
| Full WSL game `make valgrind` matrix | **Bruce/Jack** | Box may lack full game tree + valgrind; note in handoff |

**Freeze gate checklist:**

1. [x] STYLE_REVIEW written (this file)
2. [x] Chapter Exercises rewritten (A)
3. [x] LESSON+TASK on named scenario folders (B)
4. [x] `make check` on scenario + Bruce micro Makefiles (C)
5. [x] FIXES_APPLIED + HANDOFF_FOR_BRUCE written
6. [ ] Then Bruce: stub/solution correctness, `make bad` / valgrind notes

---

## 5. Success criteria for this review file

- [x] `/workspace/learnc-rpg/out/docs/pedagogy/STYLE_REVIEW_MUSTFIX_BATCH.md` exists
- [x] What's good / must-fix / nice-to-have / freeze readiness
- [x] Absolute paths throughout
