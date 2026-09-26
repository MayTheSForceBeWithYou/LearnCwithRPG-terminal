# Style / pedagogy review — Ch08 & Ch09

**Reviewer role:** box-only review for Monk (sync + deepen) before Bruce freeze.  
**Date:** 2026-09-25 PDT  
**Primary tree reviewed:** `/workspace/learnc-rpg/live/` (WSL extract)  
**Also consulted:** `/workspace/learnc-rpg/out/` (LESSON+TASK authoring drafts), `/workspace/learnc-rpg/repo/chapters/` (older book snapshot), patches under `/workspace/learnc-rpg/out/chapters/`

**Preferences applied:** adult/specific tone; **RPG** (avoid the J-prefixed genre tag); prefer `LESSON.md` depth (thin README = incompleteness); otto-didact shape (advance organizer → foundations → worked example with **rejected wrong reading** → check yourself → takeaways); side-folder drills, not production vandalism; punch-list edits over rewriting in this pass.

---

## 0. Inventory (absolute box paths)

### Live mirror (canonical-for-review)

| Path | Role |
| ---- | ---- |
| `/workspace/learnc-rpg/live/chapters/ch08-a-real-game-loop.md` | Live chapter (Exercises already retargeted) |
| `/workspace/learnc-rpg/live/chapters/ch09-the-camera.md` | Live chapter (spotlight + Exercises retargeted) |
| `/workspace/learnc-rpg/live/code/ch08/practice/` | Glyph / render seam / key-dispatch drills |
| `/workspace/learnc-rpg/live/code/ch09/practice/` | Fundamentals + page-snap / dead-zone / clamp-and-center |
| `/workspace/learnc-rpg/live/docs/pedagogy/AUDIT_WEAK_EXERCISES.md` | Prior audit |
| `/workspace/learnc-rpg/live/docs/pedagogy/NOTES_FOR_BRUCE.md` | Older out-oriented Bruce note (partially stale vs live names) |

**Live Ch08 practice**

- `/workspace/learnc-rpg/live/code/ch08/practice/README.md`
- `/workspace/learnc-rpg/live/code/ch08/practice/ANALOGIES_FOR_CHAPTER.md` *(present — do not re-author)*
- `/workspace/learnc-rpg/live/code/ch08/practice/01-glyph-table/` — thin `README.md` (~258 B), `main.c`, `SOLUTION.md`, `solutions_main.c`, Makefile
- `/workspace/learnc-rpg/live/code/ch08/practice/02-render-boundary/` — thin `README.md` (~317 B), stubs, `SOLUTION.md`, Makefile (**no** separate leak target)
- `/workspace/learnc-rpg/live/code/ch08/practice/03-key-dispatch/` — thin `README.md` (~243 B), `main.c`, `SOLUTION.md`, Makefile

**Live Ch09 practice**

- `/workspace/learnc-rpg/live/code/ch09/practice/README.md`
- `/workspace/learnc-rpg/live/code/ch09/practice/ANALOGIES_FOR_CHAPTER.md` *(present)*
- `/workspace/learnc-rpg/live/code/ch09/practice/00-fundamentals/` — five micro-programs + `solutions/`
- `/workspace/learnc-rpg/live/code/ch09/practice/01-page-snap/` — denser `README.md` (~2.4 KB), `main.c`, `SOLUTION.md`
- `/workspace/learnc-rpg/live/code/ch09/practice/02-dead-zone/` — denser `README.md` (~2.5 KB)
- `/workspace/learnc-rpg/live/code/ch09/practice/03-clamp-and-center/` — denser `README.md` (~2.5 KB)

### Out drafts (reference depth — not yet the live shape)

| Path | Role |
| ---- | ---- |
| `/workspace/learnc-rpg/out/code/ch08/practice/{01-glyph-table,02-render-boundary}/LESSON.md` + `TASK.md` | Full otto-didact LESSONs (~4.8–4.9 KB) |
| `/workspace/learnc-rpg/out/code/ch09/practice/{01-page-snap,02-dead-zone,03-clamp-center-edge-cases}/LESSON.md` + `TASK.md` | Full LESSONs; **folder name differs** from live `03-clamp-and-center` |
| `/workspace/learnc-rpg/out/chapters/ch08-exercises-patch.md` | Proposed Exercises (largely **already applied** on live, with different numbering/ex3 ship feature) |
| `/workspace/learnc-rpg/out/chapters/ch09-exercises-patch.md` | Proposed Exercises (live went further: visibility ship Ex3, `00-fundamentals`) |
| `/workspace/learnc-rpg/out/NOTES_FOR_MONK.md`, `/workspace/learnc-rpg/out/NOTES_FOR_BRUCE.md` | Sync / freeze notes |

**No** `LESSON.md` / `TASK.md` anywhere under `/workspace/learnc-rpg/live/code/ch0{8,9}/practice/`. Live uses `README.md` + `SOLUTION.md` (+ `solutions_*.c`).

---

## 1. What’s good

### Architecture / lasting path (both chapters)

- Side-folder doctrine is **live and enforced in chapter Exercises**, not only in notes:
  - Ch08: blockquote forbids deleting `render_present` / calling `refresh()` from `draw_world`; Ex3 is seam check in practice.
  - Ch09: spotlight explicitly says page-snap is alternate math; Ex2 requires practice folders; Ex3 ships `camera_is_on_screen` instead of pasting page-snap over `camera_center_on`.
- Practice index READMEs (`…/practice/README.md`) say plainly: do not mutate game `render_*` / `camera.c`.
- Arrow keys (Ch08) and visibility API (Ch09) are correctly treated as **lasting product** work, not throwaways.
- **RPG** wording: live Ch08 Next-up and Ch09 body use RPG; no `RPG` hits in live chapters (only a “not RPG” reminder inside Ch08 ANALOGIES).

### Writing / teaching tone (chapters)

- Adult/specific: Ch08 opener assumes prior pointer work and talks about opaque interfaces as a deliberate wall, not a “fun first game.” Ch09 treats coordinate spaces and clamp bounds as engineering discipline.
- Reference games (*Legend of Zelda*) appear as **examples**, not as a RPG genre label.
- Ch09 ANALOGIES bank is strong (rulers, photograph/poster, leash, elevator vs hallway) and already flags the bad exercise redesign.

### Ch09 practice (closer to bar)

- Scenario READMEs (`01`–`03`) have a usable teaching spine: Learning goal → why/why-not production → what you’ll write → instructions → hints → Expected understanding.
- Explicit **rejected path** language appears (don’t replace game camera; don’t delete `clamp` to silence warnings) especially in `01-page-snap` and `03-clamp-and-center`.
- `00-fundamentals/` is a good addition live has that the earlier `out/` batch lacked — clamp/center/bounds/conversion/visibility reps before scenarios.
- Harnesses print tables / EXPECT columns; solutions exist beside stubs.

### Ch08 practice (direction right, prose incomplete)

- Themes match the audit: glyph table, present seam, key-dispatch table.
- `ANALOGIES_FOR_CHAPTER.md` already encodes rejected wrong readings (scatter glyphs; delete one-line `render_present`).
- Stubs + `solutions_*.c` give a clear “fill TODO / compare” loop.

### Completeness vs chapter Exercises (mapping)

| Chapter exercise (live) | Practice home | Status |
| ----------------------- | ------------- | ------ |
| Ch08 Ex1 glyph | `01-glyph-table` | Linked |
| Ch08 Ex2 arrows (game) + optional `03-key-dispatch` | game + `03` | Linked |
| Ch08 Ex3 seam | `02-render-boundary` | Linked |
| Ch08 Ex4 HUD sketch | chapter only | OK (paper; Ch12 later) |
| Ch09 Ex1 reading check | `00-fundamentals/02_center` | Linked |
| Ch09 Ex2 practice folder | `00`–`03` | Linked |
| Ch09 Ex3 `camera_is_on_screen` | lasting game path | Linked + solutions sketch |
| Ch09 Ex4 mode-enum sketch | `02-dead-zone` + NOTES | Linked |

---

## 2. Must-fix gaps (punch-list for Monk)

Prefer **concrete file edits** on the live tree after sync. Do not freeze for Bruce until these are done or explicitly waived.

### A. Ch08 drill prose is incomplete (thin README → deepen to LESSON)

Preference: **LESSON.md depth**. Live drill READMEs are ~250–320 bytes — task blurbs, not lessons. Flag as incompleteness.

| # | Edit |
| - | ---- |
| A1 | For each of `01-glyph-table`, `02-render-boundary`, `03-key-dispatch`: **add** `LESSON.md` (keep or slim `README.md` to a one-liner pointer). Port structure from `/workspace/learnc-rpg/out/code/ch08/practice/01-glyph-table/LESSON.md` and `…/02-render-boundary/LESSON.md`. Required sections: advance organizer (“what this lesson asks”), foundations, worked example with **rejected wrong reading**, check yourself (2–4 Qs), key takeaways, optional lookup. |
| A2 | Add `TASK.md` (or a clearly labeled “Task” section) that says build/check only and **Do not edit** `code/ch08` game sources. Out’s `TASK.md` files are usable templates. |
| A3 | `03-key-dispatch`: there is **no** out LESSON — author one in the same otto-didact shape; fold the switchboard analogy from `ANALOGIES_FOR_CHAPTER.md`. |
| A4 | Fold 1–2 sentences from `/workspace/learnc-rpg/live/code/ch08/practice/ANALOGIES_FOR_CHAPTER.md` into each LESSON intro (do not leave analogies as an unread sibling forever). |

### B. `make check` promised by chapter, missing in live Makefiles

| # | Edit |
| - | ---- |
| B1 | Live Ch09 Exercises Ex2 says `` `make && make check` `` for `01-page-snap` (and implies harness checks). Live Makefiles under `/workspace/learnc-rpg/live/code/ch09/practice/0{1,2,3}*/Makefile` have **no** `check` target (only `all` / `clean` / `run`). **Add** `check:` that runs the binary and fails on nonzero / missing “EXPECTED” / assert path — mirror `/workspace/learnc-rpg/out/code/ch09/practice/*/Makefile`. |
| B2 | Same for Ch08 practice Makefiles (also no `check`). Chapter Solutions point at `SOLUTION.md`; still add `make check` so “complete the drill” is mechanical. Stubs already have `check(...)` helpers in `main.c`. |
| B3 | Align practice README / chapter command blocks after B1–B2 so every documented `make check` exists. |

### C. Ch08 render-boundary exhibit is weaker than the lesson needs

| # | Edit |
| - | ---- |
| C1 | Out’s `02-render-boundary` has a intentional **leak** target (`game_leak.c` / `make leak`) as a *reading* exhibit. Live only has the correct stub + `fake_refresh` declared-but-forbidden. **Add** a second target (or a short LESSON subsection + optional `make leak`) so learners *see* the wrong wiring without editing the game. Do **not** instruct deleting `render_present` in `code/ch08/`. |
| C2 | In `LESSON.md` for 02, state the rejected wrong reading in chapter-exercise language: “delete `render_present` and call `refresh` from `draw_world`.” |

### D. Dual-tree / naming drift (sync before Bruce)

| # | Edit |
| - | ---- |
| D1 | Decide **live names win**: keep `03-clamp-and-center` (live), do not rename to out’s `03-clamp-center-edge-cases` unless Bruce prefers; update `/workspace/learnc-rpg/out/` notes/patches to match live to avoid a second sync fight. |
| D2 | Port **edge-case depth** from out LESSON `03-clamp-center-edge-cases` into live `03-clamp-and-center/LESSON.md` (or deepen README): `MAP < VIEW` pin-at-zero, odd `VIEW/2` truncation, why high of `MAP` is wrong, three-line clamp low-branch-wins. Live README table is good but thinner on `MAP < VIEW`. |
| D3 | Refresh `/workspace/learnc-rpg/live/docs/pedagogy/NOTES_FOR_BRUCE.md` (or replace with a short live-accurate note): it still describes `out/code/ch09/.../03-clamp-center-edge-cases` and pre-live exercise numbering. |
| D4 | Treat `/workspace/learnc-rpg/out/code/ch09/practice/` as **reference prose**, not a second copy to sync blindly over live stubs (live has `00-fundamentals`, different `main.c` TODO shape, `SOLUTION.md`). |

### E. Harness / flags consistency (small but blocking “green check”)

| # | Edit |
| - | ---- |
| E1 | Live practice uses `-std=c17 -Wall -Wextra -Wpedantic -g` without `-Werror`. Out drafts used `-std=c11 -Wall -Wextra -Werror`. Pick one standard for practice (recommend match chapter game Makefiles on WSL) and document it in each practice `README.md`. |
| E2 | After adding `check`, run all drills on WSL and paste a one-line pass matrix into `docs/pedagogy/` or the practice index README (Monk). |

### F. Do not reintroduce old vandalism (guardrail)

| # | Edit |
| - | ---- |
| F1 | Grep live Solutions + SOLUTION.md for `delete.*clamp`, `delete.*render_present`, `replace.*camera_center_on` as student instructions — should remain **warnings against**, not steps. Spot-check after LESSON ports. |
| F2 | Keep `ANALOGIES_FOR_CHAPTER.md` files; they are already added — only fold snippets into LESSONs (see A4). |

---

## 3. Nice-to-have

| # | Idea |
| - | ---- |
| N1 | Rename scenario `README.md` → `LESSON.md` on Ch09 `01`–`03` after adding Check yourself / Takeaways / explicit Rejected wrong reading headings (content is ~70% there). |
| N2 | Add `TASK.md` beside Ch09 scenarios so LESSON (teach) / TASK (do) split matches Ch08 out drafts. |
| N3 | Ch09 `00-fundamentals/README.md` (~685 B): optional short LESSON or per-file one-paragraph advance organizers; micro-drills can stay thinner than scenarios. |
| N4 | Ch08 chapter Solutions Ex1 currently points only at `SOLUTION.md` — after LESSON exists, add “read LESSON then TASK” in the Exercises blockquote. |
| N5 | Reconsider whether Ch09 still wants the peek-ahead `snprintf` HUD exercise — live **dropped** it (good vs audit P3); leave dropped unless Bruce wants a Ch12 forward pointer as a non-coding callout. |
| N6 | Out patch’s conceptual Ex6–7 (corner `@` slide; world coords passed to `render_draw_tile`) — optional add to live Exercises as paper items; not required if fundamentals + Ex1 cover them. |
| N7 | `CameraMode` enum: keep side-only (agree with NOTES_FOR_BRUCE). No game wiring this batch. |
| N8 | Book-wide RPG pass (DESIGN/README/title screens) is outside Ch08/09 freeze — track in audit Batch D/E, not this file. |

---

## 4. Ready to leave frozen for Bruce after Jack’s later batches?

### Verdict: **Not yet — freeze after Monk completes §2 A–E (or waives with Bruce sign-off)**

| Area | Freeze-ready? | Why |
| ---- | ------------- | --- |
| Live **chapter** Exercises / spotlight / Next-up (Ch08+Ch09) | **Mostly yes** | Side-drill doctrine landed; RPG wording OK; lasting features chosen well (arrows, visibility). Tiny follow-ups only (N4, N6). |
| Live **Ch09** practice scenarios | **Almost** | Prose usable; blocked on `make check` (B1) and optional LESSON heading pass (N1). Edge-case deepen (D2) before calling “pedagogy complete.” |
| Live **Ch08** practice drills | **No** | Thin READMEs fail the LESSON-depth preference; no otto-didact check-yourself/takeaways; no leak exhibit; no `make check`. |
| `out/` vs `live/` dual trees | **No until D1–D4** | Bruce must not get two conflicting folder names / exercise histories. |
| ANALOGIES | **Yes** | Already present; only fold-in work remains. |

**Recommended freeze gate (Monk checklist):**

1. [ ] Ch08 `01`–`03` each have `LESSON.md` (+ `TASK.md` or equivalent) meeting otto-didact sections (§2 A).  
2. [ ] All practice Makefiles used by chapter text expose working `make check` (§2 B).  
3. [ ] Ch08 `02` has an explicit wrong-path exhibit for reading (§2 C).  
4. [ ] Docs/notes use **live** folder names; out drafts marked reference-only (§2 D).  
5. [ ] One WSL pass matrix for Ch08+Ch09 practice (§2 E2).  
6. [ ] Then tag practice+chapter Exercises as frozen for Bruce; Jack’s later chapter batches do not reopen Ch08/09 drill themes without a new audit ticket.

Until gate 1–5 clear: **do not** tell Bruce practice sources are frozen.

---

## 5. Sync checklist (Monk fills when pulling box ↔ WSL)

Use this when mirroring; check boxes on the WSL side after copy.

```
[ ] live/chapters/ch08-a-real-game-loop.md  ↔  chapters/ch08-a-real-game-loop.md
[ ] live/chapters/ch09-the-camera.md        ↔  chapters/ch09-the-camera.md
[ ] live/code/ch08/practice/                ↔  code/ch08/practice/
[ ] live/code/ch09/practice/                ↔  code/ch09/practice/
[ ] live/docs/pedagogy/                     ↔  docs/pedagogy/
[ ] out/docs/pedagogy/STYLE_REVIEW_CH08_CH09.md  →  docs/pedagogy/ (copy review into repo)
[ ] Do NOT overwrite live practice with out/code/ch09/practice/ blindly
[ ] After deepen: re-run make && make check in every drill folder
```

---

## 6. Success criteria for this review file

- [x] `/workspace/learnc-rpg/out/docs/pedagogy/STYLE_REVIEW_CH08_CH09.md` exists
- [x] (1) What’s good
- [x] (2) Must-fix gaps (punch-list)
- [x] (3) Nice-to-have
- [x] (4) Freeze readiness for Bruce after Jack’s later batches
- [x] Absolute box paths reported throughout
