# Design Document: "Learn C by Building a RPG"

**A specification for an interactive, incremental tutorial series.**

---

## 0. Read This First

**You are building a TUTORIAL, not a game.**

The artifact you produce is a course: a set of markdown lesson files, supporting code
snapshots, and exercises that teach a human to write the game themselves. You are the
curriculum author. The learner is the programmer.

You will inevitably write code — every chapter contains code examples, and each chapter
ships a working snapshot so the learner can check their work. That is expected and
required. What is forbidden is *racing ahead*: writing chapter 12's game, then
back-filling lessons that explain it. Build the course in order, one chapter at a time,
and never let a chapter contain code that depends on a concept the learner has not yet
been taught.

If at any point you find yourself thinking "let me just implement the battle system and
then explain it," stop. That inverts the entire purpose.

---

## 1. Goals

| # | Goal |
|---|---|
| G1 | Teach C — the real language, including the parts that hurt — to someone who wants to learn by building something they care about. |
| G2 | Produce a genuinely playable RPG in the spirit of *Final Fantasy I* / *Dragon Warrior*: overworld, towns, dungeons, NPCs, turn-based combat, inventory, magic, shops, save/load, a final boss. |
| G3 | Keep the game runnable at the end of **every single chapter**. Never a broken intermediate state. |
| G4 | Develop and play entirely inside WSL2 running Arch Linux. |
| G5 | Let the learner make the creative and technical decisions that shape *their* game, at structured checkpoints. |

### Anti-goals

- ❌ Not a C++ tutorial. Not "C with classes." Not a game-engine tutorial.
- ❌ No Unity, Godot, Unreal, or any engine. No third-party dependency beyond the
  narrow set chosen at Checkpoint A.
- ❌ No 300-line code dumps with "as you can see" underneath.
- ❌ No chapter that ends with code that does not compile.
- ❌ Not a reference manual. Concepts arrive when the game needs them, not alphabetically.

---

## 2. Learner Profile

Assume the learner:

- Can program in *something* (Python, JS, Java) but is new to C, or has bounced off C before.
- Is comfortable in a terminal, but not an expert in Linux toolchains.
- Knows what a compiler is but not what a linker does, and has never seen a `Makefile`.
- Has never manually managed memory and will absolutely leak, double-free, and
  overrun buffers. **Plan for this.** Treat memory bugs as scheduled curriculum, not
  accidents.
- Wants to *finish something*. Motivation is the scarcest resource — every chapter must
  end with a visible, tangible improvement to the game.

Do not condescend, and do not assume expertise. When you use a term for the first time
(translation unit, undefined behaviour, dangling pointer), define it inline in one sentence.

---

## 3. Target Environment

**Windows host → WSL2 → Arch Linux → terminal or WSLg window.**

The tutorial must include a Chapter 0 that gets the learner from "I have WSL Arch
installed" to "I compiled and ran a C program." Do not assume any toolchain exists.

### Packages to install (verify names against current Arch repos before publishing)

*(Checked 2026-08-23: `sdl2` no longer exists as a package — it is now
`sdl2-compat`, which provides the SDL2 API on top of SDL3. Everything else
below still resolves.)*

```bash
sudo pacman -S --needed base-devel gdb valgrind git pkgconf
# Terminal-mode track:
sudo pacman -S --needed ncurses
# Graphical track (only if chosen at Checkpoint A):
sudo pacman -S --needed sdl2-compat sdl2_image sdl2_ttf sdl2_mixer mesa vulkan-icd-loader
```

### WSL-specific guidance that MUST appear in Chapter 0

1. **Keep the project inside the Linux filesystem** (`~/projects/...`), never under
   `/mnt/c/...`. Cross-filesystem I/O is dramatically slower and mangles Unix file
   permissions, which breaks `make` in confusing ways.
2. **Verify WSLg** if the graphical track is chosen: check that `$DISPLAY` is set and
   that a trivial SDL2 window opens. Note that `wsl --update` from PowerShell fixes a
   large fraction of "no window appears" reports.
3. **Terminal choice matters** for the ncurses track. Windows Terminal handles colour
   and box-drawing characters well; the legacy console host does not. Mention `$TERM`
   and what to do if it is `dumb` or unset.
4. **Editor workflow**: VS Code with the WSL extension, or Neovim inside WSL. Mention
   both; don't mandate either.
5. **Audio** (if the optional sound chapter is attempted) routes through WSLg's
   PulseAudio bridge and generally "just works" on current WSL builds — but call out
   that this is the most fragile part of the stack.

Add a **WSL Troubleshooting appendix** collecting these plus: stale kernel symptoms,
`pacman-key --init` / keyring failures on freshly imported rootfs images, and clock-skew
breaking TLS during `pacman -Sy`.

---

## 4. Deliverable Structure

Produce this tree:

```
rpg-tutorial/
├── README.md                  # Course overview, how to use, chapter index
├── PROJECT_CHOICES.md         # The Choices Ledger — see §6
├── chapters/
│   ├── ch00-setup.md
│   ├── ch01-first-program.md
│   ├── ch02-...
│   └── ...
├── code/
│   ├── ch01/                  # Complete, compiling snapshot at END of chapter 1
│   ├── ch02/
│   └── ...                    # One directory per chapter, cumulative
├── assets/
│   └── maps/, data/           # Introduced from the file-I/O chapter onward
└── appendices/
    ├── a-wsl-troubleshooting.md
    ├── b-c-pitfalls.md
    ├── c-gdb-and-sanitizers.md
    ├── d-glossary.md
    └── e-stretch-goals.md
```

### Snapshot rules

- `code/chNN/` is the **exact state of the project after the learner completes chapter NN**.
- Every snapshot must compile cleanly with `-Wall -Wextra -std=c17` and run.
- `code/chNN/` is built by copying `code/ch(NN-1)/` and applying only that chapter's changes.
  The learner should be able to `diff -r code/ch07 code/ch08` and see exactly the chapter's work.
- Snapshots are a **safety net**, not the lesson. The chapter text teaches; the snapshot
  rescues a learner who got stuck.

---

## 5. Pedagogical Rules

These are the rules that make or break the tutorial. Follow them strictly.

### 5.1 Incrementality

- Each chapter begins from the previous chapter's end state. Explicitly say so:
  *"Pick up where chapter 6 left off. If you got lost, copy `code/ch06/` and start there."*
- **Never** present a finished subsystem. Build the battle system across three chapters:
  first a single attack that always hits for fixed damage, then variable damage and
  turn order, then magic and items. The learner should watch it grow.
- The first version of anything is allowed to be naive. Say so out loud —
  *"This is bad. It works, and we'll fix it in chapter 14 when you know about function
  pointers."* Deliberate, acknowledged imperfection is a teaching tool; silent
  imperfection is a bug.

### 5.2 How to present code

- **Show diffs and functions, not files.** When modifying `battle.c`, show the function
  being changed with a line of context, not all 400 lines.
- Full-file listings only for files under ~40 lines, or on first creation.
- Every non-obvious line gets a comment or a following prose explanation. Prefer prose;
  don't turn the source into an essay.
- Use fenced code blocks with the language tag (` ```c `, ` ```bash `, ` ```make `).
- Mark where code goes unambiguously: *"Add this to `entity.h`, below the `Vec2`
  definition."* Never "add this to the file."

### 5.3 Chapter rhythm

Every chapter has this shape, in this order:

1. **Where we are** — one paragraph recap, and what the game can currently do.
2. **The problem** — what's missing or broken, framed as something the learner wants.
3. **C concept spotlight** — the new language feature, taught in isolation with a tiny
   standalone example (a 15-line program the learner can compile separately). This is
   the "learn C" half and it must not be skipped.
4. **Apply it** — use the concept to build the game feature, incrementally.
5. **Compile and run** — exact commands, expected output, a screenshot-in-text of what
   they should see.
6. **What just happened** — a short "under the hood" note. Memory layout diagrams (ASCII)
   are extremely valuable here; use them for pointers, structs, the stack, and the heap.
7. **Common errors** — 2–4 real error messages the learner is likely to hit, verbatim,
   with the cause and fix. This section saves more learners than any other.
8. **Exercises** — 2–4, ordered easy → hard. At least one must be open-ended and
   game-design flavoured. Provide solutions in a collapsed `<details>` block.
9. **Next up** — one-sentence hook.

### 5.4 Teaching C honestly

- **Do not hide undefined behaviour.** When the learner writes something that is UB,
  name it, explain why the standard leaves it undefined, and show what can go wrong.
- **Teach memory ownership as a discipline, not a rule.** Every `malloc` gets a documented
  owner and a matching `free`. Introduce the convention early: functions named `*_create`
  pair with `*_destroy`.
- **Introduce the debugger before the learner desperately needs it** — chapter 10 or so,
  right after the first `malloc`. Cover `gdb` basics (`break`, `run`, `bt`, `print`) and
  `-fsanitize=address,undefined`. A learner who can read an ASan report is unstoppable.
- Explain *why* C is like this where it illuminates: why strings are terminated by a NUL
  byte, why arrays decay to pointers, why `struct` padding exists. History and machine
  reality make the weirdness memorable.

### 5.5 Tone

Direct, warm, occasionally funny. Confidence-building without cheerleading. It is fine to
say "this next part is genuinely tricky and most people get it wrong the first time."
It is not fine to say "this is easy!"

---

## 6. Interaction Protocol — Asking the Learner

The learner has explicitly asked to be consulted. This is a **required** feature of the
tutorial, not an optional nicety.

### 6.1 The Choices Ledger

Maintain `PROJECT_CHOICES.md` at the repo root. After every checkpoint, append the
question asked, the options offered, and the answer chosen, with a date. Every later
chapter that depends on a choice must read from this ledger and reference it explicitly:

> *You chose an MP pool back in Checkpoint D, so we'll implement `spend_mp()` here.
> (If you'd picked FF1-style spell charges, this function would look quite different —
> see the sidebar at the end of this chapter.)*

Without this ledger, a long branching tutorial drifts into incoherence by chapter 15.

### 6.2 How to ask

- **Stop and wait.** Present the question, then end your turn. Do not ask a question and
  then answer it yourself in the same breath.
- **2–4 options**, each with a one-line trade-off — including the *pedagogical* trade-off
  ("this one teaches you more about pointers; this one gets you playing sooner").
- **Always offer a default.** If the learner says "you pick" or "whatever's best,"
  take the default, record it, and move on without further questions.
- **Never block on creative choices.** If the learner doesn't want to name their kingdom,
  generate a name, tell them it's a placeholder, and note where to change it later.
- Ask one checkpoint's worth of questions at a time. Do not front-load all six checkpoints
  into chapter 0 — the learner won't have the context to answer well.

### 6.3 The Checkpoints

---

#### **Checkpoint A — Technology Stack** *(before Chapter 1)*

**Ask:**

1. **Rendering approach?**
   - **(Default) Terminal + ncurses** — authentic to *Rogue*/early *DW*, zero graphics-API
     distraction, all attention on C itself. Runs anywhere in WSL with no WSLg dependency.
   - **SDL2 with tile sprites** — real 2D graphics, closer to the NES look, but adds
     surface/texture/event-loop concepts on top of learning C, and depends on WSLg working.
   - **Start ncurses, migrate to SDL2 later** — Chapter 8 builds a renderer abstraction
     precisely so this stays possible. Recommend this to the ambitious.

2. **Prior C experience?** None / Some / Rusty. Adjusts pacing and how much of the
   spotlight sections to expand.

3. **Pacing preference?** Thorough (every concept explored) vs. Brisk (concepts introduced
   as needed, deeper dives in appendices).

**Downstream impact:** determines the `render.h` implementation, the package list in
Chapter 0, and whether Chapter 9's camera chapter talks in cells or pixels.

---

#### **Checkpoint B — World & Story** *(before Chapter 13)*

By now the learner has a walkable map and wants it to *mean* something.

**Ask:**

1. **Setting** — high fantasy kingdom / post-apocalyptic ruins / island archipelago /
   something of their own invention.
2. **Tone** — earnest & heroic, melancholy, comedic, or grim.
3. **The hook** — what has gone wrong in the world? Offer 3 concrete premises plus "mine."
4. **Names** — kingdom, starting village, the hero. Offer generated placeholders.
5. **Party structure** — solo hero (*Dragon Warrior*) or a party of 3–4 (*Final Fantasy*)?
   Note the trade-off plainly: a party makes battles far richer and teaches array-of-structs
   management, but roughly doubles the complexity of every combat chapter.

**Downstream impact:** all dialogue, map names, enemy roster, and the entire shape of
Part V. This is the single highest-leverage checkpoint — do not rush it.

---

#### **Checkpoint C — Combat Design** *(before Chapter 15)*

**Ask:**

1. **Turn order** — fixed by party order (simplest), speed-stat sorted (recommended
   default), or an ATB/time-based queue (most complex, teaches priority queues).
2. **Damage formula** — simple (`atk - def`), classic FF1-style with variance, or
   let the learner design one from a described toolkit.
3. **Encounter style** — random encounters on a step counter (default), or visible
   enemies wandering the map (teaches basic AI/pathfinding, more work).
4. **Difficulty philosophy** — forgiving, classic-brutal, or tunable via a config file.

---

#### **Checkpoint D — Magic & Progression** *(before Chapter 18)*

**Ask:**

1. **Magic resource** — a single MP pool (default, simple), FF1-style per-level spell
   charges (authentic, teaches 2D arrays nicely), or per-spell cooldowns.
2. **Class system** — single hero archetype, fixed classes chosen at start, or
   learn-anything. Only offer classes 2 and 3 if the learner chose a party at Checkpoint B.
3. **Levelling curve** — table-driven from a data file (default, teaches file parsing) or
   formula-driven.

---

#### **Checkpoint E — Save Format** *(before Chapter 20)*

**Ask:**

1. **Text-based save** (key = value or simple INI) — human-readable, debuggable, teaches
   parsing and validation. **Default.**
2. **Binary `fwrite` of structs** — fast and simple to write, but a superb vehicle for
   teaching struct padding, endianness, and why "just dump the struct" is a trap. Choose
   this if the learner wants to understand memory layout deeply.
3. **Both**, with a versioned header — the "real software" answer.

Whichever is chosen, the chapter must cover save-file *validation*: never trust a file
on disk.

---

#### **Checkpoint F — Endgame & Scope** *(before Chapter 23)*

**Ask:**

1. **Final boss and ending** — let the learner write it. Offer three shapes if they want
   scaffolding.
2. **Which stretch goals** interest them (from Appendix E): audio, sprite animation,
   a boat/vehicle that opens new terrain, NPC pathfinding, a bestiary, New Game+,
   an SDL2 port if they started on ncurses.
3. **Distribution** — do they want a chapter on packaging the game so a friend can run it?

*Answered 2026-08-22: core-six scope (Bellhollow Mine and The Unfinished
Tower cut), Vex is fought and put down, all four stretch goals go in
Appendix E, and packaging becomes its own Chapter 24. See
`PROJECT_CHOICES.md`.*

---

## 7. Curriculum

### Overview

| Ch | Title | C Concepts | Game Milestone |
|----|-------|-----------|----------------|
| 0 | Setting Up WSL Arch | toolchain, pacman, `gcc`, editors | Environment ready |
| 1 | Your First Program | `main`, `printf`, compile/link pipeline, `-Wall` | Title screen prints |
| 2 | Data and Input | types, `int`/`char`/`float`, `fgets` vs `scanf`, casting | Name your hero, see stats |
| 3 | Decisions and Loops | `if`/`switch`/`while`/`for`, functions, scope, return values | One-round coin-flip duel |
| 4 | Many Files, One Program | headers, include guards, translation units, `make` | Project skeleton, `make run` |
| 5 | Arrays and the Map | arrays, 2D arrays, indexing, bounds (and going out of them) | A world map on screen |
| 6 | Structs and Enums | `struct`, `enum`, `typedef`, composition | Hero drawn at a position |
| 7 | Pointers | addresses, `*`/`&`, pass-by-reference, `NULL`, const-correctness | Walk around with collision |
| 8 | **A Real Game Loop** ⚑ | opaque interfaces, separating logic from display | Raw input, no Enter key |
| 9 | The Camera | modular arithmetic, clamping, coordinate spaces | A world larger than the screen |
| 10 | The Heap | `malloc`/`free`, ownership, leaks, ASan, `gdb` | Maps allocated at runtime |
| 11 | Reading Files | `FILE*`, `fopen`/`fgets`, parsing, error handling | Edit maps without recompiling |
| 12 | Strings | NUL termination, `str*` functions, buffer overruns, word wrap | NPCs speak; text boxes |
| 13 | **A World to Explore** ⚑ | arrays of structs, lookup tables | Towns, dungeons, warps, NPCs |
| 14 | State Machines | function pointers, `typedef` of function types, dispatch tables | Clean EXPLORE/DIALOG/MENU modes |
| 15 | **Numbers That Fight** ⚑ | RNG and seeding, integer overflow, pure functions, unit tests | Damage math you can trust |
| 16 | The Battle Loop | struct arrays, sorting, state within state | Winnable random encounters, XP, levels |
| 17 | Menus and Inventory | dynamic arrays, `realloc`, growth strategies | Carry and use items |
| 18 | **Magic and Equipment** ⚑ | data-driven design, function-pointer effects, bit flags | Spells, gear, stat modifiers |
| 19 | Shops and Economy | more state machines, input validation | Gold, shops, inns |
| 20 | **Saving the Game** ⚑ | serialisation, struct padding, endianness, versioning | Save and load |
| 21 | Data-Driven Content | tokenising, tables, hot-reloading content | Add enemies without recompiling |
| 22 | Making It Solid | assertions, defensive coding, `valgrind`, UB hunting | Clean run under sanitizers |
| 23 | **The Ending** ⚑ | boss phases, ending sequence, final polish | Boss, ending, credits |
| 24 | Packaging and Distribution | static vs dynamic linking, `make install`, runtime deps | A friend can run it |

⚑ = chapter preceded by a checkpoint.

*(Chapter 24 added 2026-08-22 at Checkpoint F. Packaging was originally
folded into Chapter 23; the learner chose to split it so the ending is not
competing for space with linker mechanics. Chapter 23's concept column moved
from packaging to the boss fight and ending accordingly.)*

### Chapter Notes

**Chapters 1–3** are pure C with the game as flavour. Resist adding architecture here.
The learner needs `printf` to work before they need modularity.

**Chapter 4** is the most important early chapter and the one most tutorials botch.
Explain the compilation model properly: preprocessing → compilation → assembly → linking,
what a `.o` file is, why headers declare and `.c` files define, and what "undefined
reference to `foo`" actually means. Write the `Makefile` by hand and explain every line;
do not hand over a magic build script.

**Chapter 7** is the pointer chapter — the crux of the whole course. Budget extra length.
Use ASCII memory diagrams heavily. Motivate pointers by making the learner *fail* first:
have them write `move_player(Player p)` in chapter 6, watch it not work, and only then
learn why. That failure is worth a thousand words of explanation.

**Chapter 8** introduces the renderer abstraction:

```c
/* render.h — the only thing the rest of the game knows about drawing */
bool render_init(void);
void render_shutdown(void);
void render_clear(void);
void render_draw_tile(int x, int y, TileType t);
void render_draw_text(int x, int y, const char *s);
void render_present(void);
InputEvent input_poll(void);
```

Backed by `render_ncurses.c` or `render_sdl.c`, selected in the `Makefile`. Teach this as
the payoff of headers: game logic never learns whether it's drawing to a terminal or a
window. This is also what makes a later ncurses → SDL2 migration a weekend project instead
of a rewrite.

**Chapter 10** must include a deliberate memory bug. Have the learner write a leak, then
find it with ASan. Have them use freed memory, then watch it crash under a sanitizer and
*not* crash without one — the lesson that "it ran fine" proves nothing is the single most
valuable thing C teaches.

**Chapter 14**'s function-pointer dispatch table should be motivated by the pain of the
`switch` statement written in chapter 13. Let the ugly version exist for a chapter so the
refactor feels earned.

**Chapter 15** introduces the first tests. A tiny hand-rolled assertion harness (~30 lines,
no framework) is enough. Damage formulas are perfect for this: pure functions, easy to
verify, and the learner immediately sees why testable design matters.

**Chapter 22** should have the learner run the *entire game* under `valgrind` and
`-fsanitize=address,undefined` and fix everything that falls out. Realistically, bugs
planted across the previous 21 chapters will surface here. That is by design and should be
stated: *"You've been writing C for twenty chapters. Let's find out what you got away with."*

---

## 8. Target Architecture

The tutorial converges on roughly this module set. Modules appear when their chapter
arrives — never before.

```
src/
├── main.c          # entry, top-level game loop            (ch 1, grows throughout)
├── game.h/.c       # game state, mode switching            (ch 14)
├── render.h        # display abstraction                   (ch 8)
├── render_ncurses.c / render_sdl.c                         (ch 8)
├── input.h/.c      # event polling, key mapping            (ch 8)
├── map.h/.c        # tiles, collision, loading             (ch 5, 10, 11)
├── entity.h/.c     # player, NPCs, positions               (ch 6, 7)
├── dialog.h/.c     # text boxes, word wrap                 (ch 12)
├── battle.h/.c     # combat state machine                  (ch 16)
├── combat_math.h/.c# pure damage/hit functions             (ch 15)
├── party.h/.c      # heroes, stats, levelling              (ch 16)
├── inventory.h/.c  # dynamic item array                    (ch 17)
├── magic.h/.c      # spells and effects                    (ch 18)
├── shop.h/.c       # buying and selling                    (ch 19)
├── save.h/.c       # serialisation                         (ch 20)
└── data.h/.c       # content tables from disk              (ch 21)
```

Keep the final program under ~5,000 lines. It must remain readable by the person who
wrote it. If a chapter's design pushes past that, simplify the design, not the explanation.

*(Raised from ~3,500 to ~5,000 on 2026-08-22 at the learner's instruction, with the
game at 2,937 lines and four chapters remaining.)*

---

## 9. Code Standards for All Examples

- **C17**, compiled with `gcc -std=c17 -Wall -Wextra -Wpedantic -g`.
- No compiler warnings in any snapshot. Warnings are errors in spirit; say so in ch 1.
- `snake_case` for functions and variables, `PascalCase` for `typedef`'d types,
  `SCREAMING_CASE` for macros and enum constants.
- Prefix public functions with their module: `map_load`, `battle_begin`, `party_heal`.
- 4-space indentation, no tabs, ~90-column soft limit (code blocks must not wrap in markdown).
- Every `.h` gets an include guard; explain `#pragma once` exists but prefer the portable form.
- Always check the return value of `malloc`, `fopen`, and every allocating library call.
  The tutorial should model this from the very first allocation, without exception.
- Prefer `snprintf` over `sprintf`, `fgets` over `gets` (which does not exist any more —
  explain why it was removed; it's a great story).
- Keep functions under ~50 lines. When one grows past that in a chapter, split it and
  say why.

---

## 10. Definition of Done

Before considering the tutorial complete, verify every item.

*Status as of 2026-08-23: every box below verified from disk. The build and
test sweep is reproducible — see §"Build & Verify" in `CLAUDE.md`.*

*The concept-ordering rule was audited mechanically across all 25 snapshots
(first appearance of `struct`, `enum`, pointer dereference, `malloc`/`free`,
`fopen`, the `str*` family, function pointers, `realloc`, bit flags,
`assert` and `<stdarg.h>`, each against its spotlight chapter). One genuine
early use exists: `strlen` appears in Chapter 11's `map.c` before Chapter
12's strings spotlight. It is glossed inline where it is used and called out
explicitly in Chapter 11's "Next up" — acknowledged rather than silent,
which §5.1 permits. No unacknowledged violations found.*

- [x] Every chapter file exists and follows the §5.3 structure.
- [x] Every `code/chNN/` snapshot compiles with zero warnings under the standard flags.
- [x] Every snapshot runs and is playable to the extent that chapter claims.
- [x] `diff -r code/chNN code/ch(NN+1)` shows only the changes that chapter taught.
- [x] No chapter uses a C concept before its spotlight section introduces it
      (one acknowledged exception, documented above).
- [x] All six checkpoints are present, each asking and then *waiting*.
- [x] `PROJECT_CHOICES.md` exists and every branching chapter references it.
- [x] Every chapter has a "Common errors" section with real, verbatim compiler output.
- [x] All exercises have solutions in collapsed `<details>` blocks.
- [x] All five appendices are written.
- [x] The final game has: overworld, ≥2 towns, ≥1 dungeon, NPCs with dialogue, random
      encounters, turn-based combat, levelling, inventory, magic, equipment, shops,
      save/load, a final boss, and an ending.
- [x] `README.md` lists all chapters with one-line descriptions and estimated time.

---

## 11. Suggested Execution Order

For the agent building this course:

1. Write `README.md` and the skeleton of `PROJECT_CHOICES.md`.
2. **Run Checkpoint A. Wait for answers.** Do not proceed until answered.
3. Write Chapter 0, verified against the answers.
4. Write chapters 1–12 sequentially. After each: build the snapshot, compile it, run it,
   fix it, *then* move on. Never batch multiple chapters before verifying.
5. **Run Checkpoint B. Wait.** Then chapters 13–14.
6. **Run Checkpoint C. Wait.** Then chapters 15–17.
7. **Run Checkpoint D. Wait.** Then chapters 18–19.
8. **Run Checkpoint E. Wait.** Then chapters 20–22.
9. **Run Checkpoint F. Wait.** Then chapter 23 and the appendices.
10. Final pass: work the §10 checklist end to end.

If the learner asks to skip ahead or change an earlier choice, update
`PROJECT_CHOICES.md` and flag which already-written chapters are affected. Do not
silently leave the course inconsistent.
