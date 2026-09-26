# Instructional Depth Audit — Learn C with RPG

**Date:** 2026-09-25 (PT)  
**Author:** Bruce (curriculum structure / technical depth)  
**Pedagogy target (Nathan):** more reading + more exercises in **separate practice folders**; avoid one-off game-codebase futzing that will not persist on the real architecture path. Students should **engrain** skills (clamping, map/camera math, ownership, etc.) with drills — not copy-paste into the editor to "save time."

**Status legend:** `DONE` = fixed on disk this pass · `PARTIAL` = chapter prose or one exercise improved · `TODO` = still weak · `OK` = already durable enough

---

## 1. Executive summary

The course already has strong bones: chapter rhythm (DESIGN.md §5.3), common-errors sections, collapsed solutions, and a continuous game. The recurring weakness is **exercise shape**:

| Pattern | What it looks like | Why it fails Nathan's bar |
|---------|-------------------|---------------------------|
| **A. Knob-twiddle** | Change a `#define`, rebuild, watch | Confirms curiosity; does not engrain skill |
| **B. One-off rewrite** | Replace lasting API with a throwaway variant in-tree | Student ships the wrong architecture forward, or reverts without learning |
| **C. Peek-ahead futz** | Use next chapter's API inside the game with "take it on faith" | Skips the drill; encourages paste-and-pray |
| **D. Sketch-only open end** | Design question with no code path | Fine as *one* item; insufficient as the only hard work |
| **E. Missing side drills** | Concept spotlight describes math; exercises only edit `game` | Spotlight knowledge evaporates without reps |

**Already strong (keep as models):**

- ch05 ex3 — standalone out-of-bounds probe (not the game)
- ch07 ex3 — standalone `swap` drill
- ch10 ex1–2 — sanitizer/leak experiments tied to ownership
- ch12 ex2 — dialog round-trip *test* (proves a property)
- ch15 ex2 — monotonicity property test
- ch16 simulation harness — measurement over intuition

**Priority fix order:**

1. **P0 — ch09 camera** (`DONE` this pass): deepest "math skill" chapter; ex3 was classic Pattern B.
2. **P0 — ch05 arrays / map indexing** (`TODO`): clamping & bounds are prerequisites; needs practice folder.
3. **P1 — ch07 pointers** (`TODO`): only one real drill; need more standalone pointer drills.
4. **P1 — ch08 render/input** (`TODO`): ex1–3 are mostly Pattern A/B on the game binary.
5. **P1 — ch10 heap** (`PARTIAL` later): good sanitizer exercises; still needs alloc/free drills without the full map module.
6. **P2 — ch11–14** file/string/world/state: mix of durable and shallow; add practice modules per chapter.
7. **P2 — ch15–20** combat/inventory/shop/save: stronger on properties; still too game-only for pure math (damage, flee, inventory ops).
8. **P3 — ch00–04, ch21–24, appendices**: earlier chapters are setup-heavy (OK); later chapters need packaging/debug drills more than game futzing.

---

## 2. Technical notes — skills that must be taught (and drilled)

### 2.1 Clamping

```c
int clamp(int v, int lo, int hi);  /* assume lo <= hi */
```

Teach explicitly:

- **Precondition:** if `lo > hi`, behaviour must be defined by the API (assert, swap, or return `lo`). Silent nonsense is how camera bugs ship.
- **Idempotence:** `clamp(clamp(v, lo, hi), lo, hi) == clamp(v, lo, hi)`.
- **Edge cases:** `v == lo`, `v == hi`, `v == lo-1`, `v == hi+1`, extreme `INT_MIN`/`INT_MAX` (overflow if someone writes `hi - lo + 1` carelessly).
- **Camera upper bound:** `max_cam_x = MAP_W - VIEW_W` — not `MAP_W`, not `MAP_W - 1`. Off-by-one here paints fake walls.
- **When VIEW > MAP:** upper bound goes negative; clamp is the wrong tool unless you also guard configuration.

### 2.2 World vs screen coordinates

| Space | Origin | Extent | Who owns it |
|-------|--------|--------|-------------|
| World | map top-left | `MAP_W` × `MAP_H` | `Player`, NPCs, warps, map data |
| Screen / view | viewport top-left | `VIEW_W` × `VIEW_H` | `render_draw_*` only |
| Camera | world position of screen `(0,0)` | clamped to `[0, MAP-VIEW]` | `Camera` |

Conversions (following camera):

```
screen = world - camera
world  = screen + camera
```

Discipline: **never** pass bare `x` across a module boundary. Name `world_x` / `screen_x`. Route conversions through one module (`camera_*`).

Visibility cull:

```
visible iff camera.x <= world_x < camera.x + VIEW_W
         && camera.y <= world_y < camera.y + VIEW_H
```

This becomes mandatory once NPCs exist (ch13) — teach it in ch09 practice, wire it when entities multiply.

### 2.3 Camera centering math

```
cam.x = clamp(target_x - VIEW_W/2, 0, MAP_W - VIEW_W)
```

Integer `/2` truncates toward zero — for odd view widths the "center" is biased; call that out. Page cameras use `/` and `%` as a pair; negative `%` is a late trap.

### 2.4 Map / indexing math (ch05+)

Row-major: `offset = y * width + x`. Teach `y` outer, `x` inner; off-by-one on `<` vs `<=`; negative indices. Practice folder should beat this in before the camera depends on it.

### 2.5 What "durable integration" means here

A chapter exercise that edits the game should:

1. Leave behind an API or behaviour **later chapters keep**, or
2. Be clearly labeled temporary *and* paired with a practice drill that owns the learning.

Page-scrolling as a replacement for `camera_center_on` in `code/ch09/` fails (1) and historically failed (2).

---

## 3. Findings by chapter

### ch00 — Setup — `OK` / light `TODO`

| Ex | Pattern | Notes |
|----|---------|-------|
| 1–3 | A / toolchain | Appropriate for setup |
| 4 | D sketch | Fine |

**Recommend:** optional `code/ch00/practice/` with a "break the compiler on purpose" checklist (already mostly inline). Low priority.

### ch01 — First program — `OK`

Banner/title exercises are on-theme. Ex3 preprocessor peek is good reading. No practice folder needed yet.

### ch02 — Data and input — `DONE`

| Ex | Issue |
|----|-------|
| 1 | Knob on `hp = -5` — OK warm-up |
| 2 | Add `gold` to the one-file program — persists into structs later only by re-typing |
| 3 | Cast removal — good conceptual |
| 4 | Sketch |

**Recommend:** `practice/` with format-string drills (`%d` vs `%f`), integer division traps, and a tiny "stat block printer" standalone — not only editing `main.c`.

### ch03 — Decisions and loops — `DONE`

Ex3 "add a loop duel variant" is useful but still game-futz. **Recommend:** standalone loop drills (sentinel, countdown, validation loops) in `code/ch03/practice/`.

### ch04 — Many files / make — `OK` / light `TODO`

Ex1–3 are excellent toolchain drills (make, `.o` deletion, include guards). Ex4 sketch is fine. Optional: practice Makefile with two modules and a deliberate missing header.

### ch05 — Arrays and the map — `DONE` **P0**

| Ex | Verdict |
|----|---------|
| 1 | Good — forces UB confrontation |
| 2 | Pattern A (add a row) — shallow |
| 3 | **Model exercise** — standalone |
| 4 | Sketch toward camera — good priming |

**Gaps:** no progressive drills on row-major indexing, fill/clear, finding a tile, counting walkable cells, clamping indices.  
**Recommend:** `code/ch05/practice/` with 5+ programs; deepen prose on flat layout; replace ex2 with a practice pointer + a durable `map_tile_at` introduction if not already present by ch09.

### ch06 — Structs and enums — `DONE`

Ex1 gold field / ex2 enum rank are light. Ex3 return-success sketch is good.  
**Recommend:** standalone struct-init and enum-switch drills; party-member array practice that foreshadows ch16 without rewriting the game.

### ch07 — Pointers — `DONE` **P1**

Ex3 `swap` is the model. Ex1 input leftover is excellent. Need more: pointer arithmetic on arrays, `const` correctness, returning pointers to locals (anti-pattern demo), multiple out-params.  
**Recommend:** `code/ch07/practice/` (swap, minmax out-params, walk an array via pointer, broken dangling return).

### ch08 — Real game loop — `DONE` **P1**

| Ex | Issue |
|----|-------|
| 1 | Change wall glyph — Pattern A |
| 2 | Arrow keys — durable and good |
| 3 | Call `refresh` directly — architecture lesson, but still game edit |
| 4 | HUD sketch — becomes peek-ahead |

**Recommend:** practice: mock "render" that records calls into an array (test double); input mapping table drill. Keep arrow keys as the game-path exercise.

### ch09 — The camera — `DONE` **P0**

| Ex (old) | Issue |
|----------|-------|
| 1 | Knob-twiddle VIEW size |
| 2 | Peek-ahead `snprintf` HUD in game |
| 3 | **Pattern B** — replace lasting center camera with page scroll |
| 4 | Dead-zone sketch only |

**Fixes applied this pass:**

- Deepened chapter prose (coordinate spaces, clamp edges, visibility, `/` `%`, odd VIEW bias).
- Exercises rewritten: required practice drills + durable integration (`camera_is_on_screen` / visibility helper on the lasting API) + design sketch that does not demolish `camera_center_on`.
- Added `code/ch09/practice/` with progressive standalone programs + `solutions/`.
- Page-scroll and dead-zone live as **drills**, not as the shipped camera.

### ch10 — The heap — `DONE` **P1**

Ex1–2 sanitizer work is excellent. Ex3 `map_create_sized` is durable.  
**Recommend:** `practice/` with malloc/free pairing, NULL checks, intentional UAF under ASan, growable buffer — without linking the whole game.

### ch11 — Reading files — `PARTIAL`

Ex1 corruption battery is strong. Ex2 comments durable. Ex3 error-channel redesign is good architecture.  
**Recommend:** practice parsers on tiny `.map` fixtures in `practice/fixtures/`.

### ch12 — Strings — `DONE`

Ex2 round-trip test is a model. Ex1 clamp interaction good.  
**Recommend:** more standalone string drills (length, copy, truncation, word wrap) in `practice/` before dialog integration.

### ch13 — World to explore — `DONE`

Ex1 add area is content futz (acceptable once). Ex2 `world_validate` is durable gold. Ex3 generic lookup is advanced/good.  
**Recommend:** practice: linear search, table-driven dispatch, validation harness on fake area tables.

### ch14 — State machines — `DONE`

Ex2 compile-time mode coverage is excellent. Ex3 GAME_OVER durable.  
**Recommend:** practice: tiny state machine in 50 lines (menu/walk/quit) with a transition table — not only editing `game.c`.

### ch15 — Numbers that fight — `DONE`

Property tests are strong. Ex1 "Test your nerve" is mild game futz but teaches wiring.  
**Recommend:** `practice/` pure combat_math drills with a tiny test main (no ncurses).

### ch16 — Battle loop — `DONE`

Simulation harness is exemplary. Ex1 defend-state lifetime is durable systems thinking. Ex3 flee XP is small durable fix.  
Keep; add practice only if battle setup is too heavy to experiment.

### ch17 — Menus and inventory — `DONE`

Ex2 swap-with-last order sensitivity is subtle and good.  
**Recommend:** practice inventory ops as a standalone ADT with tests (add/remove/count/full).

### ch18 — Magic and equipment — `DONE`

Ex1–2 extend tables — durable data-driven path.  
**Recommend:** practice effect-dispatch table (function pointers) standalone.

### ch19 — Shops and economy — `DONE`

Ex3 "don't save inside `shop_rest`" is excellent architecture. Ex1 selling ordering is durable.  
**Recommend:** practice: transactional buy/sell on a fake inventory (check-then-commit).

### ch20 — Saving — `DONE`

Ex2 version bump / ex3 atomic replace are strong.  
**Recommend:** practice: read/write a tiny binary blob with version field; corrupt and detect.

### ch21 — Data-driven content — `DONE`

Ex2 sort dependency is sharp.  
**Recommend:** practice CSV/line parsers with fixtures.

### ch22 — Making it solid — `DONE`

Sanitizer/assert/fuzzer exercises match the chapter. Keep; optional practice harness templates.

### ch23 — Ending — `DONE`

Format-attribute and phase-line exercises are on-point.

### ch24 — Packaging — `DONE`

ldd / DESTDIR / `--where` are real packaging drills.

### Appendices — `OK`

Reference material; not exercise-driven. Glossary/stretch goals fine. Optional: link appendix C from ch10 practice.

---

## 4. Prioritized fix list

| Pri | Item | Status |
|-----|------|--------|
| P0 | Genre-tag → RPG rename (all tree except `.git`) | **DONE** |
| P0 | Full audit document | **DONE** (this file) |
| P0 | Deepen ch09 prose + rewrite exercises | **DONE** |
| P0 | `code/ch09/practice/` + solutions | **DONE** |
| P0 | ch05 practice folder + deepen bounds prose | **DONE** |
| P1 | ch07 practice folder (pointer drills) | **DONE** |
| P1 | ch08 practice (render test double / input map) + trim knob ex | **DONE** (filled Monk stubs; exercises rewritten) |
| P1 | ch10 practice (heap drills) | **DONE** |
| P1 | ch11–12 practice (parse / string / wrap) | **DONE** |
| P2 | ch13–14 practice (tables / FSM) | **DONE** |
| P2 | ch15–20 practice ADTs (combat, inventory, shop txn, save blob) | **DONE** |
| P2 | ch02–03 early drills | **DONE** |
| P3 | ch21 parsers; cross-links from chapters to practice | **DONE** (01-spell-line + Monk 02) |
| P3 | Integrate Monk analogy / alternate scenarios when present | TODO (none on disk at audit time) |

---

## 5. Practice folder convention (going forward)

```
code/chNN/practice/
  README.md           # ordered list of drills, build commands
  01_short_name.c     # standalone; #include only libc (+ maybe a tiny .h)
  02_...
  Makefile            # builds all drills
  solutions/
    01_short_name.c
    ...
```

Chapter markdown should end exercises with:

> Complete drills 01–0N in `code/chNN/practice/` before the integration step.  
> Do not paste solutions into the game to skip the reps.

Game-path exercises must either extend lasting APIs or be marked *optional experiment* and live under `practice/`.

---

## 6. Coordination with Monk

At audit time no Monk-authored files were present under the project. When analogies, alternate scenarios, or extra drills appear:

- Prefer **adding** scenarios under `practice/` (e.g. `04b_dead_zone_story.c`) rather than replacing numbered drills.
- Do not overwrite deepened technical sections for tone-only edits; merge analogies into "Where we are" / spotlight sidebars.
- Update this file's status table when a chapter batch lands.

---

## 7. Verification notes (this pass)

- Rename script: 24 files, ~30 substitutions; `grep -ri <old-genre-tag> --exclude-dir=.git` → **0** hits for the old genre tag.
- Banner spacing normalized to `*           UNTITLED RPG               *` (38-char interior).
- Repo folder name `LearnCwithRPG-terminal` left unchanged.
- No commit / push performed.


## 8. Monk integration (ch09) — 2026-09-25

Monk landed otto-didact practice modules under `code/ch09/practice/`:

- `01-page-snap/`, `02-dead-zone/`, `03-clamp-and-center/`
- `ANALOGIES_FOR_CHAPTER.md`
- `docs/pedagogy/NOTES_FOR_BRUCE.md`, `AUDIT_WEAK_EXERCISES.md`

Bruce kept those, added `00-fundamentals/` (progressive clamp→visibility
check programs), merged `practice/README.md`, folded the "two rulers"
analogy into the chapter, and aligned exercises with both tracks.
Recommendation accepted: **no `CameraMode` enum in the game for ch09**;
variants stay side-only.

## 9. Progress log

| When (PT) | Batch | Notes |
|-----------|-------|-------|
| 2026-09-25 ~23:00 | Rename + audit + ch09 full + ch05 practice scaffold | Genre-tag→RPG done (0 hits). ch09 deepened, `camera_is_on_screen` on lasting path, practice merged with Monk. ch05 `practice/` created; ch05 prose deepen + remaining chapters still TODO. |
| 2026-09-25 ~23:20 | TECH PASS | Genre 0; loader solutions.c; appendix B/C links; solutions compile 83/0; artifacts cleaned. See docs/pedagogy/TECH_PASS_BRUCE.md. Ready for PR. |
| 2026-09-25 ~23:40 | ch02–03,06,21–24 practice+ex | Early drills; ch06; ch21 01-spell-line + Monk 02 kept; ch22–24 practice. ch09/11/15 not wiped. Appendices reference-only. |
| 2026-09-25 ~23:35 | ch16–20 practice+ex | Battle/inventory/magic/shop/save side drills + practice-first exercises. ch09 untouched. Later batch covered ch02–03 and ch21–24. |
| 2026-09-25 ~23:30 | ch12–15 practice+ex | Strings/wrap drills; world tables+validate; FSM/fnptr drills; combat pure-math practice. Exercises practice-first. ch09 untouched. ch16+ TODO. |
| 2026-09-25 ~23:25 | ch11 practice+ex | Parser drills + exercise rewrite. ch09 left untouched. ch12+ TODO. |
| 2026-09-25 ~23:15 | ch05 deepen+ex; ch07/08/10 practice+ex | ch05 flat-layout prose; ch07 five pointer drills; filled ch08 Monk stubs; ch10 four heap drills; exercises rewritten to side-folders. Later batch covered ch11 practice. |


## 10. Remaining gaps (final tech pass)

- Optional: cross-link appendices C/B to ch10/ch22 practice
- Optional: fold Monk analogies into chapter prose bodies
- Optional: ch00/ch01/ch04 micro-practice if desired
- Re-check concurrent Monk trees (ch11/15/21) after sync
- CI: compile all practice solutions across chapters
