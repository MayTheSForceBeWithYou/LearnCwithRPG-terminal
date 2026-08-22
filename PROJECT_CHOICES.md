# The Choices Ledger

This file is the authoritative record of every decision the learner has made
at a checkpoint. Later chapters reference entries here explicitly instead of
re-deciding or silently assuming. See `DESIGN.md` §6 for the protocol this
file follows.

Each entry: the question asked, the options offered, the answer chosen, and
the date.

---

## Checkpoint A — Technology Stack *(before Chapter 1)*

**Status:** Answered 2026-08-22.

**Q1: Rendering approach?**
Options: Terminal + ncurses (default) / SDL2 with tile sprites / start ncurses, migrate to SDL2 later.
**Chosen: Terminal + ncurses.**
Impact: `render.h` is backed by `render_ncurses.c` from Chapter 8 onward. Chapter 0's
package list installs `ncurses`, not SDL2/mesa/vulkan. Chapter 9's camera talks in
character cells, not pixels.

**Q2: Prior C experience?**
Options: None / Some / Rusty.
**Chosen: None.**
Impact: Spotlight sections run at full length, nothing assumed. Every new term gets
an inline one-sentence gloss on first use, no skipping.

**Q3: Pacing preference?**
Options: Thorough / Brisk.
**Chosen: Thorough.**
Impact: Concepts get explored in depth in the chapter body itself, rather than being
pushed to appendices for a quick read-through.

---

## Checkpoint B — World & Story *(before Chapter 13)*

**Status:** Not reached.

---

## Checkpoint C — Combat Design *(before Chapter 15)*

**Status:** Not reached.

---

## Checkpoint D — Magic & Progression *(before Chapter 18)*

**Status:** Not reached.

---

## Checkpoint E — Save Format *(before Chapter 20)*

**Status:** Not reached.

---

## Checkpoint F — Endgame & Scope *(before Chapter 23)*

**Status:** Not reached.
