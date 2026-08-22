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

**Status:** Answered 2026-08-22.

**Context:** `CONTENT.md` (the learner's own content bible) pre-supplied a complete
answer to this checkpoint. Per the learner's instruction on 2026-08-22, the checkpoint
was still asked directly rather than auto-adopted; the learner confirmed all three
answers below. `CONTENT.md` is therefore authoritative for game *content* (names,
dialogue, tables), while this ledger and `DESIGN.md` remain authoritative for choices
and curriculum.

**Q1: Setting, tone, and premise?**
Options: adopt CONTENT.md as written / same premise with an earnest-heroic tone /
a different setting entirely.
**Chosen: adopt CONTENT.md as written.**

- **Title:** *Some Assembly Required* — *A Quest in Four Pieces*
- **Setting:** The kingdom of Hollis. High fantasy administered by a mid-sized civil
  service that is doing its best.
- **Premise:** King Aldric the Reasonably Adequate died heirless, willless, and
  crownless — the Crown of Hollis had been sundered into four fragments. Its wards
  were the only thing keeping the wilds out of the farmland for three hundred years,
  and they are failing. The Guild of Heraldic Reassembly hires the hero to recover
  the fragments, because they are available and cheap.
- **Tone:** Dry, understated, faintly bureaucratic. The world is absurd; the
  characters are sincere. Nobody in the game knows they are in a comedy. No memes,
  no anachronisms, no winking at the player. Dial the comedy down from the
  Unfinished Tower onward so the Act III reveal lands.
- **The hook / twist:** The official account blames a dark sorcerer named **Vex**.
  There was never a sorcerer. **Perrin Culch**, a junior royal accountant, tripped on
  a rug while carrying the crown between vaults, panicked, invented a villain, and
  filed the report. Eleven years of concentrated belief in a kingdom coming loose at
  the seams made Vex real. He is looking for the fragments too, because he has been
  told his whole existence that he broke the crown and would like to know why.
- **Locations (gated in sequence):** Grubbin Vale (village) → Wetwood Barrow
  (fragment I) → Mudwick (town) → The Sump (fragment II) → Castle Hollis (capital
  hub) → Bellhollow Mine (fragment III) → The Unfinished Tower (fragment IV) →
  The Crownless Court (final). Core vs. extended scope is decided at Checkpoint F,
  not before.

Impact: all dialogue, map names, enemy roster, and the shape of Part V. Chapter 13
onward uses these names. **Perrin Culch must be planted in Chapter 13's Castle Hollis
content** with one forgettable line about filing — the Act III reveal only lands if
the player has already met him.

**Q2: Party structure?**
Options: solo hero (*Dragon Warrior*) / party of 3–4 (*Final Fantasy*).
**Chosen: solo hero.**
Trade-off as presented: a party would teach array-of-structs management and make
battles richer, but roughly doubles the complexity of every combat chapter, requires
turn order across actors, per-member equipment/MP, and KO/revive handling — and would
invalidate `CONTENT.md`'s single-hero stat model and level tables. Solo keeps the
project under `DESIGN.md`'s ~3,500 line cap.
Impact: Chapters 15–18 target one hero. Progression is gated by key item and level,
not party composition. Checkpoint D's class-system question is constrained — per
`DESIGN.md` §6.3, options 2 and 3 (fixed classes / learn-anything) are only offered
if a party was chosen, so Checkpoint D will offer the single-archetype option.

**Q3: Hero name?**
Options: Wick (CONTENT.md default) / neutral placeholder in prose / learner picks another.
**Chosen: Wick.**
A village odd-jobs kid. Not chosen, not descended from anyone, not foretold —
competent by the end entirely through effort. The game still prompts for a name at
runtime; "Wick" is the canonical name used in written dialogue and chapter examples.

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
