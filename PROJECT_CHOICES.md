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
project under `DESIGN.md`'s line cap (~3,500 at the time of this decision; raised to
~5,000 on 2026-08-22).
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

**Status:** Answered 2026-08-22.

**Q1: Turn order?**
Options: fixed by party order / speed-stat sorted / ATB time-based queue.
**Chosen: speed-sorted by AGI, ties go to the hero** (`CONTENT.md` §9.3's model).
Impact: every combatant carries an AGI stat; Chapter 16 sorts a small array of
structs once per round. With a solo hero this is the hero versus 1–3 enemies.
AGI also drives hit chance and flee chance, so it is doing three jobs and needs
care when balancing.

**Q2: Damage formula?**
Options: simple `atk - def` / FF1-style with variance / learner designs one.
**Chosen: FF1-style with variance** (`CONTENT.md` §9.2 as written):

```
base     = (attacker.ATK + weapon.power) - (defender.DEF / 2)
base     = max(base, 1)
variance = random in [-12%, +12%] of base
damage   = max(1, base + variance)

critical: 1 in 32 -> damage * 2, ignoring defender.DEF
```

Spells ignore physical DEF entirely and use a flat power value plus the same
variance band, keeping the magic path meaningfully different from the sword path.
Impact: Chapter 15 builds these as pure functions with a hand-rolled test harness.
`CONTENT.md` notes the level table values are a starting point that **will need
playtesting** — chapters must say so rather than presenting them as tuned.

**Q3: Encounter style?**
Options offered: random on a step counter / visible wandering enemies.
**Chosen: random on a step counter, *plus* a player-facing option to turn
encounters off entirely** — as the Final Fantasy Pixel Remasters do.
Base model from `CONTENT.md` §9.3: roll each step, trigger when
`steps_since >= 12 && rand() % 100 < 22`, then reset; tunable per region.
Impact: this is a **deviation from `CONTENT.md`**, which did not include a toggle.
The toggle is read from the config file below (see Q4) and is also exposed as a
runtime toggle in the Chapter 14 menu, so the player can flip it mid-run. Chapter
16's encounter check gains an early-out when encounters are disabled.

**Q4: Difficulty philosophy?**
Options: forgiving / classic-brutal / tunable via a config file.
**Chosen: tunable via a config file.**
Impact: also a **deviation from `CONTENT.md`**, which left difficulty unstated.
A text config file (parsed with the Chapter 11 file-I/O techniques, and validated
the same way — never trust a file on disk) supplies at minimum:

| Key | Meaning |
|---|---|
| `encounters` | `on` / `off` — the Q3 toggle's default at boot |
| `enemy_damage` | multiplier on damage dealt to the hero |
| `xp_rate` | multiplier on XP awarded |
| `gold_loss_on_death` | fraction of gold lost when defeated |

Missing keys fall back to documented defaults so a missing or partial config file
is never fatal. This adds a small config module; it reuses Chapter 11's parsing
and Chapter 13's table-driven lookup rather than introducing a new technique.

---

## Checkpoint D — Magic & Progression *(before Chapter 18)*

**Status:** Answered 2026-08-22.

**Q1: Magic resource?**
Options: single MP pool (default) / FF1-style per-level spell charges / per-spell
cooldowns.
**Chosen: a single MP pool.**
`CONTENT.md` §12's MP column is used directly — Mend costs 3, Sunder costs 30.
Impact: `Player` already carries `mp`/`max_mp` from Chapter 16, so Chapter 18 spends
from it and needs no new resource machinery. The `Bottled Vim` item already in the
Chapter 17 inventory (restore 20 MP) becomes meaningful rather than vestigial. Inns
restore MP as well as HP when Chapter 19 builds them. The 2D charge array that
`CONTENT.md` flagged as "a genuinely nice teaching artefact" is **not** built — if a
later chapter wants to teach 2D arrays, it needs a different vehicle.

**Q2: Class system?**
`DESIGN.md` §6.3 constrains this: options 2 and 3 (fixed classes / learn-anything) are
only offered when a party was chosen at Checkpoint B, and Checkpoint B chose a solo
hero. The question was therefore reframed as "does the lone hero specialise?"
Options offered: learns everything on schedule / chooses a focus partway through.
**Chosen: learns everything on schedule.**
Matches `CONTENT.md` §12's "solo hero learns everything on the level table". Impact:
no branching progression, no risk of a player locking themselves out, and every
playthrough sees all thirteen spells. Spell acquisition is driven purely by level.

**Q3: Levelling curve?**
Options: table-driven from a data file (default) / keep as C tables / formula-driven.
**Chosen: table-driven from data files.**
Impact: the level table currently lives as a hardcoded `static const LevelRow
level_table[]` in `party.c` (added Chapter 16), and the spell table will be added the
same way in Chapter 18. **Both move to `assets/` in Chapter 21**, parsed with the
Chapter 11 file-I/O techniques and validated the same way — never trust a file on
disk. This matters concretely: Chapter 16's simulation showed the curve needs
playtesting, and a data file means retuning without a rebuild.

Sequencing agreed at this checkpoint:
- **Chapter 18** builds spells as a C table of function-pointer effects. `CONTENT.md`
  §12 explicitly asks for this rather than a `switch`, calling it "the payoff for
  Chapter 14's dispatch-table lesson — call it back explicitly."
- **Chapter 21** moves the level table *and* the spell table out to `assets/`,
  keeping the same in-memory shapes so nothing above the loader changes.

---

## Checkpoint E — Save Format *(before Chapter 20)*

**Status:** Answered 2026-08-22.

**Q1: Save format?**
Options: text `key = value` (default) / binary `fwrite` of structs / both with a
versioned header.
**Chosen: text, `key = value`.**
Impact: saves are human-readable and repairable by hand, and Chapter 20 reuses the
Chapter 11 parsing and Chapter 17 config techniques rather than teaching new syntax —
so the chapter's real subject is *serialisation design and validation*, not I/O
mechanics. A version field still goes in the file so a future format change is
detected rather than misread (see Q3).

**What this choice costs, recorded deliberately:** `DESIGN.md` §6.3 offered binary
partly because it is "a superb vehicle for teaching struct padding, endianness, and
why 'just dump the struct' is a trap." Declining it means **Chapter 20 no longer
teaches struct padding or endianness**, and `DESIGN.md` §7's curriculum table lists
both as Chapter 20 concepts. Chapter 20 must therefore cover them as an explicit
sidebar — *why we are not doing it this way* — rather than dropping them silently.
That keeps the curriculum promise without building a format the learner did not want.

**Q2: When can the player save?**
Options: at inns only (Dragon Warrior style) / anywhere from the menu / both, with a
suspend slot.
**Chosen: at inns only.**
Matches `CONTENT.md` §15, which already describes inns as "full HP/MP restore and the
save point, Dragon Warrior style." Impact: `shop_rest` already exists and already
reports success, so saving hooks onto it — but per Chapter 19's exercise 3, the save
call belongs in the *caller* (which holds the whole `Game`) rather than inside
`shop_rest`, which currently knows about only a `Shop` and a `Player`. Resting and
saving being one action gives inns real weight and makes defeat meaningful.

**Q3: Behaviour on a corrupt or outdated save?**
Options: refuse and start fresh / load what is valid and default the rest / refuse but
keep the bad file.
**Chosen: refuse to load, explain why, start fresh.**
Impact: every field is validated on load and a partial load is never performed — the
same discipline `map_load` (Chapter 11) and `config_load` (Chapter 17) already apply
to untrusted files, now applied to a file the game itself wrote. A version field is
checked first; a mismatch is reported as a version problem rather than a parse error.
The failure message must name the specific problem, not just "save corrupt".

---

## Checkpoint F — Endgame & Scope *(before Chapter 23)*

**Status:** Not reached.
