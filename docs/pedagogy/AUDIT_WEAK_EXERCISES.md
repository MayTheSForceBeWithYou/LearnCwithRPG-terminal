# AUDIT: Weak / throwaway Exercises — LearnCwithRPG-terminal

**Audience:** Monk / Jack / Bruce (planning rewrite)  
**Date:** 2026-09-25 PT  
**Scope:** Pedagogy of chapter `## Exercises` sections — shallow, temporary, or lasting-path-diverging drills.  
**Bar (from brief):** more reading + side drills; treat readers as capable of specifics; **RPG** (avoid the J-prefixed genre tag); otto-didact-ish practice as `LESSON.md` + `TASK.md` under `code/chNN/practice/`.

---

## 0. Inventory used

| Location | What |
| -------- | ---- |
| `/workspace/learnc-rpg/repo/chapters/ch00…ch24*.md` | Full book markdown (source of truth for this audit) |
| `/workspace/learnc-rpg/repo/appendices/` | Not exercise-bearing; stretch goals noted only |
| `/workspace/learnc-rpg/repo/{DESIGN,CONTENT,README}.md` | DESIGN §5.3 still mandates 2–4 exercises + open-ended; title previously used the J-prefixed genre tag |
| `/workspace/learnc-rpg/ch08-*.md`, `ch09-*.md` | Earlier loose copies (same text as repo for Exercises) |
| `/workspace/learnc-rpg/out/code/ch09/practice/` | **Sibling executor work — DO NOT OVERWRITE** |

### Sibling practice already present (extend, do not clobber)

Observed at write time (files may still be growing — do not overwrite):

```
out/code/ch09/practice/README.md
out/code/ch09/practice/01-page-snap/               # LESSON, TASK, Makefile, page_snap.c
out/code/ch09/practice/02-dead-zone/               # LESSON, TASK, Makefile, dead_zone.c
live/code/ch09/practice/03-clamp-and-center/ # canonical live name (out draft folder differed)
```

Ch09 Ex3 (page-snap replacing `camera_center_on`, unused-`clamp` → delete) is already being fixed on this pattern. Audit does **not** own those practice files. Remaining editorial work: rewrite chapter Exercises/spotlight/solutions to *point at* practice folders instead of mutating `camera.c`.

No prior `AUDIT_WEAK_EXERCISES.md` existed; this file is new.

---

## 1. Weak-pattern catalog (use when scanning any chapter)

Score each exercise **1–4** against these anti-patterns. A chapter rewrite is warranted when ≥1 **Critical** or ≥2 **High** items sit in Exercises.

| ID | Pattern | Severity | Smell keywords / cues | Fix shape |
| -- | ------- | -------- | --------------------- | --------- |
| P1 | **Lasting-path fork** — replace core algorithm / delete helper the game still needs | **Critical** | `instead of`, `replace`, `delete the function`, `unused`, `doesn't need clamping`, `for this exercise only` | Side-folder drill; chapter text keeps production path; exercise *reads* the alternate |
| P2 | **Break-the-abstraction in situ** — strip a boundary to "see it still runs" | **High** | `try calling … directly`, `deleting … entirely`, `does the game still work` | Reading + optional practice stub backend; never leave main tree broken |
| P3 | **Peek-ahead paste** — paste API from a later chapter "on faith" | **High** | `peek ahead`, `take it on faith for now`, `Chapter N's topic` | Defer feature to that chapter, or tiny practice that teaches only the needed slice |
| P4 | **Cosmetic / knob tweak** — change a char, `#define`, or literal; learn little transferable | **Med** | `change X to Y`, `rebuild and confirm`, one-line solution | Keep as 30-second sanity check *or* promote to drill with multiple cases / asserts |
| P5 | **Delete-to-observe then restore** — good for tools, easy to leave broken | **Med** (Low early) | `delete … Put the call back`, `deliberately remove` | OK if restore is mandatory + checklist; better as harness that toggles via `#ifdef` / separate target |
| P6 | **Game-integration busywork** — content add that only proves "you can edit a table" | **Med** | `add a fourth area`, `add a spell` with no new skill | Keep 1 as lasting content path; move "how many files?" metacognition into LESSON; extra variants → practice |
| P7 | **Open-ended that mutates API in prose** — sketch that still teaches forking the shipped function | **Med** | `sketch how F would need to change`, then solution rewrites F | Sketch *new* function / mode beside shipped one; or practice TASK |
| P8 | **Genre wording drift** | **Low** (consistency) | the J-prefixed genre tag in body / title screens / DESIGN title | Prefer **RPG** (or "classic turn-based RPG"); keep FF1/DW as *reference games*, not a J-prefixed genre label |

**What is NOT weak (keep / promote into Apply it):**

- Lasting product features (arrow keys, atomic save rename, format attributes, validation at startup).
- Harness / test / sanitizer work that leaves shipped code better.
- Sketches that stay paper/comments and do not ask to fork production.
- Early "break compile / read error" drills (P5 Low) — they teach the toolchain.

**DESIGN.md tension:** §5.3 still says every chapter ends with 2–4 Exercises including an open-ended game-design one. Rewrite plan should amend DESIGN to: **Apply it** builds the lasting feature; **Exercises** = short checks + pointers into `practice/`; open-ended design can stay, but algorithmic variants leave the game tree alone.

---

## 2. Deep audit — Chapter 8 (A Real Game Loop)

**Local + repo:** `chapters/ch08-a-real-game-loop.md` · Exercises @ L475.

| # | Stem (short) | Patterns | Verdict |
| - | ------------ | -------- | ------- |
| 1 | Change `TILE_WALL` glyph `#` → `%` | P4 | **Shallow.** Teaches opacity in one line, but is throwaway mutation of lasting render code. Prefer: keep as optional "try it" callout in Apply it, **or** practice `01-glyph-table` that maps `TileType → char` in a standalone file with a print harness. |
| 2 | Add arrow-key `case`s to `input_poll` | — | **Keep on lasting path.** Real UX; belongs in Apply it or as the one "ship it" exercise. Not shallow. |
| 3 | Call `refresh()` from `draw_world`, **delete** `render_present` | P2 | **High — throwaway architecture vandalism.** Lesson is excellent (boundary loss); method is wrong. Side drill: stub `render_present` vs raw `refresh` in a tiny two-file program; or reading-only "predict what `#include` leaks." Do **not** ask readers to delete the wrapper in the game. |
| 4 | Sketch HUD field for wandering | P7-ish / OK | **Keep as sketch.** Already says don't fully implement; good. Watch for later chapters that "complete" this with peek-ahead `snprintf` (see Ch09 Ex2). |
| Next-up | "Real **RPG** worlds aren't" [fix→RPG] | P8 | Wording: use RPG. |

**Ch08 proposed practice themes** (`code/ch08/practice/`):

| Folder | Skill (same as chapter, different situation) |
| ------ | -------------------------------------------- |
| `01-glyph-table/` | `TileType` → glyph mapping; change data not call sites; assert wall≠floor≠player |
| `02-key-dispatch/` | Parallel keymap: char/`KEY_*` → enum; table-driven, not game `input.c` |
| `03-present-boundary/` | Two translation units: game draws via `present()`; "leaky" variant fails a *link* or `#include` checklist test — no ncurses in game `.c` |

**Chapter rewrite notes:** Promote Ex2 into Apply it (or leave as sole ship exercise). Replace Ex3 with pointer to `03-present-boundary`. Soften Ex1 to "optional try" or practice 01. Fix RPG in Next up.

---

## 3. Deep audit — Chapter 9 (The Camera)

**Exercises @ L464.** Known Critical already in flight.

| # | Stem (short) | Patterns | Verdict |
| - | ------------ | -------- | ------- |
| 1 | Change `VIEW_*` to 30×15 / 40; walk corners | P4 | **Med.** Good intuition pump for clamp bounds; better as print-table drill (sibling `03-clamp-and-center`) so readers see `MAP-VIEW`, odd sizes, `VIEW>MAP` without toggling the playable game. |
| 2 | HUD world coords via `snprintf` "peek ahead / faith" | P3 | **High.** Teaches HUD placement but imports Ch12. Defer stringy HUD to Ch12, or micro-practice that only formats two ints (not in `main` game). |
| 3 | Replace centering with page-snap; hint questions clamp; solution drops clamp → unused → **delete `clamp`** | **P1 Critical** | **Canonical bad exercise.** Diverges lasting path; solution reinforces deleting production safety. Sibling `01-page-snap` is the fix — chapter Ex3 must become "do practice/01; leave `camera_center_on` alone." |
| 4 | Dead-zone sketch; "**Many RPGs**" [fix→RPGs] | P7 + P8 | Open-ended OK if it does **not** rewrite `camera_center_on` in the game. Point to `02-dead-zone/` practice; say **RPGs**. |
| Spotlight | "You'll build that variant yourself in the exercises" | P1 setup | Retarget: "you'll build that variant in `practice/01-page-snap`." |

**Ch09 practice status:**

| Drill | Status (sibling; do not clobber) | Action for chapter editors |
| ----- | -------------------------------- | -------------------------- |
| `01-page-snap` | LESSON/TASK/Makefile/page_snap.c | Cross-link from Ex3 + spotlight |
| `02-dead-zone` | LESSON/TASK/Makefile/dead_zone.c | Cross-link from Ex4; keep game on center+clamp |
| `03-clamp-and-center` | LESSON/TASK/Makefile/clamp_center.c | Cross-link from Ex1; replace VIEW-tweak-in-game |

**Chapter rewrite notes (Bruce):** Solutions block for Ex3 currently teaches the anti-pattern (delete `clamp`). Rewrite solutions to: restore centering; unused-clamp warning means you edited the wrong file.

---

## 4. Full-book triage (all chapters on disk)

Severity = worst pattern in that chapter's Exercises. "Ship" = exercise that should stay on the game path.

| Ch | Title | Worst | Hot items | Ship vs side-drill |
| -- | ----- | ----- | --------- | ------------------ |
| 00 | Setup | Low | Toolchain checks; open-ended title dream | All fine as-is; light |
| 01 | First program | Low/Med | P5 delete include; `$?` / `-E` | Keep; ensure "put include back" |
| 02 | Data & input | Low | P4 hp=-5; gold add is ship | Gold → lasting; cast bug is good |
| 03 | Decisions & loops | Low | Trace / rewrite for↔while | Good; conceptual best-of-three |
| 04 | Many files | Med | P5 delete `.o` / strip include guard | Keep (make pedagogy); restore guards |
| 05 | Arrays & map | Med | P5 off-by-one run; P4 add grass row; **standalone** negative index (already side!) | Ex3 is the model — more like this |
| 06 | Structs & enums | Low | gold field ship; RANK_SS; sketch return-move | Good |
| 07 | Pointers | Low | discard-loop trace; **swap** classic drill | Ex3 swap is ideal practice DNA |
| **08** | Game loop | **High** | P2 Ex3; P4 Ex1 | See §2 |
| **09** | Camera | **Critical** | P1 Ex3; P3 Ex2; P4 Ex1 | See §3; sibling in progress |
| 10 | Heap | Med | P5 delete destroy / delete free-on-fail | Prefer sanitizer *targets* or practice OOM path; Ex3 `map_create_sized` is ship-quality |
| 11 | Reading files | Low/Med | Corrupt-file matrix is excellent; comment lines ship | Model chapter for "hostile inputs" drills |
| 12 | Strings | Low | clamp interaction; no-loss test | Strong; keep |
| 13 | World explore | Med | P6 add Mudwick; validate fn is gold | Validation → Apply it; area add OK once |
| 14 | State machines | Low/Med | validate modes; MODE_GAME_OVER | Strong systems thinking; genre-tag menu line → RPG |
| 15 | Numbers that fight | Low | flee menu; monotonic test; fixed seed | Excellent test-culture |
| 16 | Battle loop | Med | defend ship; sim harness; partial XP; "**Real RPGs**" [fix→RPGs] | P8; features mostly lasting |
| 17 | Menus & inventory | Med | P6 starting items; order-preserve; flee item | Skillful; order-preserve good either path |
| 18 | Magic & equipment | Med | P6 add spell; drain kind is real skill | Ex2 is the keeper |
| 19 | Shops | Low | sell; discount placement; rest≠save | Strong design/API judgment |
| 20 | Saving | Low | field+version; **atomic rename** ship | Ex3 is lasting production fix — promote |
| 21 | Data-driven | Low | hot-reload content; sort invariant | Strong |
| 22 | Making solid | Low | valgrind/assert/NDEBUG | Strong |
| 23 | Ending | Low | format attr; items-in-battle; phase count | Strong; intentional delete of attribute is P5 with restore |
| 24 | Packaging | Low | ldd; DESTDIR; `--where` | Strong |

**Also:** DESIGN title, README title, and title-screen strings (`UNTITLED` + old genre tag in ch04/ch05) are P8 book-wide — schedule a wording pass, not per-exercise rewrites.

---

## 5. Checklist — when auditing a chapter file that just arrived / is being rewritten

Use this as a PR review template:

1. **Locate** `## Exercises` and the `<details>` Solutions block.
2. **For each numbered item, ask:**
   - After the reader finishes, is the **shipped game** still on the chapter's taught architecture?
   - Does the solution **delete**, **replace**, or leave **unused** a symbol the Apply-it path still needs? → P1
   - Does it tell them to **tear down an abstraction** "to see it still runs"? → P2
   - Does it paste an API from a **later** chapter? → P3
   - Is the only action "change constant / glyph / one literal"? → P4
   - Does it say delete then forget to say **restore**? → P5
3. **Keyword grep** (case-insensitive) on the Exercises + Solutions text:
   - `Exercise`, `Hint`, `replace`, `delete`, `unused`, `temporary`, `try changing`, `for this exercise only`, `instead of`, `peek ahead`, `on faith`, `RPG`
4. **Spotlight / Next-up cross-links:** Do they promise "you'll build the alternate in Exercises" when that alternate must not land in `code/chNN/` game sources?
5. **Open-ended:** Sketch OK; "change function F in the game" not OK if F is production.
6. **If weak:** draft `code/chNN/practice/N-topic/{LESSON.md,TASK.md}` — different situation, **same skill**; `make` + `make check`; zero warnings; **Do not** edit game `*.c`.
7. **Chapter Exercises** become: (a) short confirmation, (b) link to practice, (c) one lasting ship feature OR paper sketch.
8. **Wording:** RPG; name reference games (Zelda rooms, Dragon Warrior inns) without a J-prefixed tag as the course genre.

---

## 6. Batch order for full-book rewrite

Priority = (harm to lasting game path) × (how soon readers hit it). Fundamentals first for *skill* drills only if they block later habits; mid-game forks first for *damage*.

### Batch A — Critical / High mid-game (do first)

| Order | Chapter | Why first | Deliverables |
| ----- | ------- | --------- | ------------ |
| A1 | **Ch09** | P1 already known; camera is dependency for all later world work | Finish practice 02/03; rewrite Ex1–4 + spotlight + solutions; RPG wording |
| A2 | **Ch08** | P2 on render boundary; teaches the habit Ch09 then abuses | practice 01–03; rewrite Ex1/Ex3; promote Ex2; Next-up RPG |
| A3 | DESIGN §5.3 + README title | Stops regenerating the old exercise shape | Amend rhythm: side drills + lasting Apply it; RPG naming |

### Batch B — Mid systems that *can* fork modules (ch10–16)

| Order | Chapter | Focus |
| ----- | ------- | ----- |
| B1 | Ch10 Heap | Convert Ex1/Ex2 delete-in-main into practice `leak-two-blocks` / `oom-partial-free` with harness; keep Ex3 sized-create on path |
| B2 | Ch14 State machines | Mostly good; add practice for `on_enter`/`on_exit` instead of only sketching against live `game.c` |
| B3 | Ch16 Battle | P8; ensure defend/partial-XP stay ship features not temporary flags |
| B4 | Ch11–Ch13, Ch15 | Light touch — extract P6 extras into practice; promote validation/tests |

### Batch C — Early fundamentals (ch00–07)

Lower damage (programs are small) but **habit-forming**. Goal: more Ch05-Ex3 / Ch07-Ex3 style standalone drills; less "tweak the only program."

| Order | Chapter | Drill themes (examples) |
| ----- | ------- | ----------------------- |
| C1 | Ch05 Arrays | Bounds / flat memory (already started with negative index) |
| C2 | Ch07 Pointers | `swap`, discard-loop stdin, success/fail move return |
| C3 | Ch02–Ch04 | Integer division cases; make graph; include-guard crash — keep toolchain P5 |
| C4 | Ch00–Ch01, Ch06 | Light edit; title-screen genre tag → RPG |

### Batch D — Late content & polish (ch17–24)

Mostly **keep**. Selective:

- Ch17–19: one content-add max per chapter on path; second variant → practice.
- Ch20 Ex3 atomic rename: **promote into Apply it** if not already required.
- Ch22–24: leave; strong.
- Book-wide P8 pass: DESIGN, README, `UNTITLED` + old genre tag prints, ch08/09/10/14/16 strings.

### Batch E — Appendix / stretch

`appendices/e-stretch-goals.md` — ensure stretch goals are framed as optional side projects, not "replace your camera/battle loop for a weekend."

---

## 7. Side-folder drill theme bank (by skill, not busywork)

Themes are **situations**, not "add another town."

| Skill cluster | Chapters | Example practice titles |
| ------------- | -------- | ----------------------- |
| Opaque boundary / link surface | 08 | glyph table; present vs refresh; fake backend `.o` |
| Camera math / clamp / snap | 09 | page-snap *(exists)*; dead-zone; clamp edge-cases |
| Ownership / partial failure | 10 | two-allocation leak; free-on-fail branch; double-free detect |
| Hostile file parsing | 11 | five corruptions as fixtures; `;` comments; start-pos format |
| String / paging invariants | 12 | no-loss pager test; width vs storage clamp |
| Table + enum + validate | 13–14 | warp/NPC validate; NULL-mode table guard |
| Pure math + tests + seed | 15–16 | monotonic damage; fixed-seed replay; flee curve |
| Growth / reorder / effects | 17–18 | shift-vs-swap remove; drain effect; ward budget |
| Commit ordering / pricing | 19 | sell commit; discount at wrong layer (negative test) |
| Versioning / atomic replace | 20 | tmp+rename; migrate v1→v2 |
| Loader invariants | 21 | unsorted spell list fixture; unknown effect name |
| Assert / sanitizer / fuzz story | 22–23 | NDEBUG tradeoff worksheet; format-attr on/off |
| Paths / packaging | 24 | `--where`; DESTDIR vs PREFIX mental model quiz |

**LESSON/TASK shape (otto-didact-ish):** LESSON = situation, worked table, wrong readings to reject, check-yourself Qs, takeaways, lookup. TASK = build/check only; "Do not edit the chapter game."

---

## 8. Success criteria for this audit file

- [x] `out/AUDIT_WEAK_EXERCISES.md` exists (this file)
- [x] Ch08 / Ch09 deep findings with exercise-level verdicts
- [x] Checklist for chapters / rewrites
- [x] Batch order A→E for full-book work
- [x] Side-folder themes (not game-integration busywork)
- [x] Sibling `out/code/ch09/practice/` noted and not overwritten
- [x] Full tree under `repo/chapters/` surveyed (ch00–ch24)

---

## 9. Suggested immediate next actions (owners)

1. **Sibling / practice owner:** leave `out/code/ch09/practice/` alone unless `make check` fails; chapter editors only cross-link.
2. **Chapter editor (Bruce):** Ch09 Exercises + Solutions + spotlight retarget; then Ch08 Ex3.
3. **DESIGN:** patch §5.3 exercise bullet + title genre-tag→RPG agreement with CONTENT tone.
4. **Do not** start Batch C polish until A1–A2 land — otherwise early chapters will re-teach "mutate the only tree."
