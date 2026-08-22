# Content Bible: *Some Assembly Required*

**Companion to `DESIGN.md`. This file supplies all game content for the tutorial.**

---

## 0. How Claude Code Should Use This File

This is the **content layer**. `DESIGN.md` is the curriculum layer. Where they touch,
`DESIGN.md` governs pedagogy and this file governs what goes *in* the game.

Read this in full before writing Chapter 12 (dialogue) and again before Chapter 15
(combat numbers). Sections 9–14 are lookup tables meant to be transcribed into the
game's data files; do not invent parallel content when a table here already covers it.

**This file pre-answers parts of two checkpoints:**

| Checkpoint | Status |
|---|---|
| **B — World & Story** | ✅ Confirmed with the learner 2026-08-22. Setting, tone, hook, names, solo hero all adopted as written. |
| **D — Magic & Progression** | ✅ Answered 2026-08-22. MP pool chosen (use the §12 MP column directly; the FF1 charge variant is NOT built). Solo hero learns everything on schedule. Level and spell tables move to data files in ch21. |

Checkpoint A: ✅ answered 2026-08-22 (ncurses, no prior C, thorough pacing).
Checkpoint C: ✅ answered 2026-08-22 — see `PROJECT_CHOICES.md`. Two deviations from
§9: encounters gain an **off toggle**, and difficulty is **tunable via a config file**.
Checkpoint D: ✅ answered 2026-08-22 — MP pool, learn-everything, tables move to data
files in ch21.
Checkpoints E and F are untouched. Ask them as specified.

**Placeholder policy:** every proper noun here is a default, not a mandate. If the
learner renames the hero or a town mid-course, update this file and
`PROJECT_CHOICES.md`, then flag which chapters contain the old name.

---

## 1. Title

> ## Some Assembly Required
> ### *A Quest in Four Pieces*

The title is the premise: a crown broken into four fragments, and a hero expected to put
it back together with no instructions and inadequate tools.

**Alternates**, if the learner wants a different flavour: *The Crown, Reassembled* (more
earnest), *Fragments & Paperwork* (leans bureaucratic), *Regnant Pending* (drier).

---

## 2. Premise

King Aldric the Reasonably Adequate died with no heir, no will, and — as it turns out —
no crown, because the crown had been sundered into four fragments and scattered across
the realm.

This is a problem. The Crown of Hollis was not ceremonial. Its wards are the only reason
the wilds stayed out of the farmland for three hundred years. The wards are now failing
in the order you'd expect: slowly, then all at once, then in your village specifically.

The official account holds that a dark sorcerer named **Vex** shattered the crown in a
bid for the throne. Broadsheets carry his likeness. Children are frightened with his name.
The Guild of Heraldic Reassembly has an entire filing cabinet devoted to him.

You are hired to recover the four fragments. Not because you are chosen, or descended
from anyone, or foretold. Because you are available, and cheap, and the Guild's budget
was allocated before anyone understood the scale of the problem.

### The Twist (Act III)

There was never a sorcerer.

The crown was broken by **Perrin Culch**, junior royal accountant, who was carrying it
between vaults, tripped on a rug, and watched three centuries of protective enchantment
bounce down a staircase. He panicked. He invented a villain. He filed the report.

And the realm believed it — thoroughly, continuously, for eleven years. Enough belief,
concentrated on one invented name, in a kingdom whose reality was already coming loose at
the seams, did what belief does under those conditions.

**Vex is real now.** He is made of what everyone imagined him to be: theatrical, cruel,
hollow at the centre, and absolutely certain of a past he never actually had. He is
looking for the fragments too, because he has been told his whole existence that he broke
the crown, and he would very much like to know why.

### Why this works

The comedy is bureaucratic; the ending is not a punchline. Vex is genuinely dangerous and
faintly tragic — a man who was invented as an excuse. Play the jokes straight and let the
last dungeon go quiet. Comedy that refuses to ever be serious gets exhausting by hour six.

---

## 3. Tone Guide

**Register:** dry, understated, faintly bureaucratic. Fantasy as administered by a
mid-sized civil service that is doing its best. The world is absurd; the characters are
sincere. Nobody in the game knows they are in a comedy.

### Do

- Undercut epic language with mundane specifics. *"The prophecy was quite clear. Well —
  clear-ish. The relevant page is water-damaged."*
- Let NPCs have small, boring problems in the middle of the apocalypse.
- Make institutions funny, not people. The Guild is ridiculous; its clerks are trying.
- Deadpan item descriptions. The joke is in the noun, not in an exclamation mark.

### Don't

- No memes, no anachronisms, no winking at the player. It's a world, not a sketch.
- No character who exists only to be a joke.
- Don't undercut the Act III reveal. From the Unfinished Tower onward, dial it down.
- Avoid exclamation marks. Dry writing dies on them.

### Voice calibration

> **Good:** "Fragment's in the barrow, they say. Nobody's checked. Nobody's *going* to
> check. That's rather what you're for."

> **Too broad:** "LOL the barrow is SO haunted, good luck buddy!!!"

> **Too flat:** "The fragment is located in the barrow to the north."

---

## 4. Cast

| Name | Role | Note |
|---|---|---|
| **Wick** | The hero (default name; player-renameable) | Village odd-jobs kid. Not chosen. Competent by the end, entirely through effort. |
| **Deputy Registrar Ammel** | Quest-giver, Guild of Heraldic Reassembly | Issues the commission. Deeply apologetic about the pay. Reappears at each act break with revised paperwork. |
| **Bess Harrow** | Grubbin Vale innkeeper | Tutorial NPC. Explains resting and saving. Thinks the whole crown business is overblown. |
| **Old Sudd** | Village elder | Delivers exposition badly, twice, contradicting himself. |
| **Perrin Culch** | Junior royal accountant | The actual cause. Encountered early as a nervous nobody in Castle Hollis; the player won't think twice about him until Act III. |
| **Vex** | The sorcerer who wasn't | Final boss. Theatrical, hollow, increasingly uncertain. |

**Plant Culch early.** He should appear in Chapter 13's Castle Hollis content with one
forgettable line about filing. The reveal only lands if the player has met him.

---

## 5. World & Progression

Six locations, gated in sequence. Solo hero means no party-composition gates — progression
is by key item and by level.

| # | Location | Type | Gate | Fragment |
|---|---|---|---|---|
| 1 | **Grubbin Vale** | Village | — | — |
| 2 | **Wetwood Barrow** | Dungeon (small) | — | **I** |
| 3 | **Mudwick** | Town | Fragment I | — |
| 4 | **The Sump** | Dungeon (medium) | Brass Key | **II** |
| 5 | **Castle Hollis** | Capital / hub | Fragment II | — |
| 6 | **Bellhollow Mine** | Dungeon (medium) | Miner's Writ | **III** |
| 7 | **The Unfinished Tower** | Dungeon (large) | Warden's Seal | **IV** |
| 8 | **The Crownless Court** | Final | All four fragments | — |

### Scope guidance

`DESIGN.md` caps the project near 3,500 lines. Content is the easiest thing to overrun.

- **Core (must ship):** Grubbin Vale, Wetwood Barrow, Mudwick, The Sump, Castle Hollis,
  The Crownless Court. That is two fragments, three settlements, two dungeons, and a
  complete story — a finishable game.
- **Extended (ship if pace allows):** Bellhollow Mine, The Unfinished Tower.
- If cutting to core, compress Fragments III and IV into a single Unfinished Tower run
  and adjust the level curve to cap around 15 instead of 20.

Decide this at Checkpoint F, not before. Don't pre-emptively shrink the world.

---

## 6. Plot Beats

**Act I — The Commission** *(levels 1–5)*
Wards fail near Grubbin Vale; something gets into the turnip field. Ammel arrives with a
commission nobody else accepted. Wick takes it, mostly for the advance. Wetwood Barrow
yields Fragment I and the first hint that the fragments are being sought by someone else —
a barrow already searched, recently, badly.

**Act II — The Search** *(levels 6–14)*
Mudwick, then the Sump. Fragment II. Castle Hollis opens: the throne room is empty, the
court is arguing about procedure, and the Vex file is enormous and internally
contradictory. The player can read it. The dates don't line up. Nobody has noticed.
Bellhollow and the Unfinished Tower follow, each with a Warden who has been guarding a
fragment far past the point of instruction.

**Act III — The Crownless Court** *(levels 15–20)*
Vex takes the fragments at the tower and departs for the Court. Culch confesses — not
dramatically, just wretchedly, at his desk, having wanted to for eleven years. The final
dungeon is quiet. Vex is waiting, and does not know what he is. The confrontation is
mostly him asking questions the player cannot answer.

**Ending.** The crown is reassembled; the wards return. Vex either unmakes himself on
realising the truth, or persists as a hollow thing that must be put down — leave this
choice to the learner at Checkpoint F. Ammel files a closing report. The pay is still bad.

---

## 7. Content → Chapter Map

Introduce content only when its chapter arrives.

| Chapter | Content from this file |
|---|---|
| 1–3 | Title screen (§1). Hero name prompt, default `Wick`. |
| 5–7 | Grubbin Vale map only. No NPCs yet. |
| 11 | Grubbin Vale + Wetwood Barrow as map files. |
| 12 | Bess Harrow, Old Sudd. Sample lines from §16. |
| 13 | Mudwick, Castle Hollis. Ammel's commission. Plant Culch. |
| 15 | Damage formula (§9.2). Wetwood enemy tier (§13). |
| 16 | Level table (§9.1). Wetwood + Sump tiers. Bogwright (§14). |
| 17 | Consumables and key items (§11). |
| 18 | Spells (§12), weapons/armour/accessories (§10). |
| 19 | Shop inventories (§15). |
| 21 | All tables move to data files. |
| 23 | Vex, the Crownless Court, the ending. |

---

## 8. Naming Conventions

Keep new content consistent with these patterns.

- **Places:** blunt Anglo-Saxon compounds. *Grubbin, Mudwick, Bellhollow, Wetwood.*
  Nothing with an apostrophe. Nothing elvish.
- **Weapons:** material or provenance, plainly stated. *Guild-Issue Longsword.*
- **Consumables:** what it is, not what it does. *Bottled Vim*, not *Mana Elixir*.
- **Spells:** one plain word, verb-like. *Mend, Ember, Sunder.* A couple of dry
  bureaucratic exceptions (*Adjourn*) but no more than two — battle text needs to stay
  readable at a glance.
- **Enemies:** mundane noun + wrong adjective. *Disgruntled Slime. Provisional Wizard.*

---

## 9. Stat Model

### 9.1 Level Table

Cap 20. Final boss tuned for 18. Solo hero, so this is the entire progression system.

| Lv | HP | MP | ATK | DEF | AGI | XP to next |
|---|---|---|---|---|---|---|
| 1 | 20 | 0 | 5 | 3 | 5 | 8 |
| 2 | 26 | 2 | 6 | 4 | 6 | 18 |
| 3 | 32 | 4 | 8 | 5 | 6 | 32 |
| 4 | 39 | 6 | 9 | 6 | 7 | 55 |
| 5 | 47 | 9 | 11 | 8 | 8 | 90 |
| 6 | 55 | 12 | 13 | 9 | 9 | 140 |
| 7 | 64 | 15 | 15 | 11 | 10 | 210 |
| 8 | 73 | 18 | 17 | 12 | 11 | 300 |
| 9 | 83 | 22 | 19 | 14 | 12 | 420 |
| 10 | 94 | 26 | 21 | 16 | 13 | 580 |
| 11 | 105 | 30 | 24 | 18 | 14 | 780 |
| 12 | 117 | 35 | 26 | 20 | 15 | 1030 |
| 13 | 130 | 40 | 29 | 22 | 17 | 1350 |
| 14 | 143 | 45 | 32 | 24 | 18 | 1750 |
| 15 | 157 | 51 | 35 | 27 | 19 | 2250 |
| 16 | 172 | 57 | 38 | 29 | 21 | 2850 |
| 17 | 188 | 63 | 41 | 32 | 22 | 3600 |
| 18 | 205 | 70 | 45 | 35 | 24 | 4500 |
| 19 | 222 | 77 | 48 | 38 | 25 | 5600 |
| 20 | 240 | 85 | 52 | 41 | 27 | — |

XP is cumulative-to-next-level, not total. Values are a starting point and **will need
playtesting** — say so in the chapter rather than presenting them as tuned.

### 9.2 Damage Formula

Reference implementation for Chapter 15. Pure functions, unit-testable.

```
base    = (attacker.ATK + weapon.power) - (defender.DEF / 2)
base    = max(base, 1)
variance= random in [-12%, +12%] of base
damage  = max(1, base + variance)

critical: 1 in 32 → damage * 2, ignore defender.DEF
```

Spells ignore physical DEF entirely and use a flat power value plus the same variance
band. Keeps the magic path meaningfully different from the sword path.

### 9.3 Hit, Flee, Encounters

- **Hit chance:** `85 + (attacker.AGI - defender.AGI)`, clamped to [10, 99].
- **Flee chance:** `50 + (hero.AGI - enemy.AGI) * 2`, clamped to [10, 90]. Bosses: 0.
- **Encounters:** step counter. Roll on each step; trigger when `steps_since >= 12` and
  `rand() % 100 < 22`. Reset on trigger. Tune per-region via a map property.
  **Checkpoint C (2026-08-22) added a player-facing toggle** to disable encounters
  entirely, as the FF Pixel Remasters do. Default comes from the config file; also
  exposed as a runtime toggle in the menu. The encounter check early-outs when off.
- **Turn order:** higher AGI acts first; ties go to the hero. Checkpoint C selected
  exactly this model, so it is now fixed rather than provisional.

### 9.4 RNG

Use a small explicit PRNG (xorshift32 is ~6 lines) rather than `rand()`. It makes seeding
deterministic, gives reproducible bug reports, and is a genuinely good teaching moment
about why library RNG quality varies. Seed from `time(NULL)` at boot; store the seed in
saves so a run can be replayed.

---

## 10. Equipment

Three slots: **weapon**, **armour**, **accessory**.

### Weapons

| Weapon | ATK | Cost | Where |
|---|---|---|---|
| Chair Leg | +2 | — | Starting gear |
| Bronze Shortsword | +6 | 90 | Grubbin Vale |
| Woodsman's Axe | +11 | 260 | Mudwick |
| Guild-Issue Longsword | +17 | 620 | Mudwick / Castle Hollis |
| Silver Rapier | +24 | 1,400 | Castle Hollis |
| Warden's Halberd | +32 | — | Bellhollow chest |
| Sunsteel Blade | +41 | 3,800 | Castle Hollis (late) |
| The Regnant | +55 | — | Forged from crown scrap, Act III |

### Armour

| Armour | DEF | Cost | Where |
|---|---|---|---|
| Work Clothes | +1 | — | Starting gear |
| Padded Jerkin | +4 | 70 | Grubbin Vale |
| Boiled Leather | +8 | 230 | Mudwick |
| Ring Mail | +13 | 580 | Mudwick |
| Guild Plate | +19 | 1,500 | Castle Hollis |
| Warden's Scale | +26 | — | Unfinished Tower chest |
| Sunsteel Harness | +34 | 4,200 | Castle Hollis (late) |

### Accessories

Where the comedy lives. Small effects, memorable names.

| Accessory | Effect | Cost | Where |
|---|---|---|---|
| Lucky Sock | +2 AGI. Unwashed, and it matters. | 120 | Grubbin Vale |
| Guild Signet | 10% shop discount | 400 | Castle Hollis |
| Ward-Charm | Halves fire damage | 700 | Mudwick |
| Ledger-Weight | +5 DEF, −2 AGI | 650 | Castle Hollis |
| Bell of Small Hours | +1 MP regen per 20 steps | 1,100 | Bellhollow |
| Culch's Spectacles | +3 to all stats. Found on his desk, Act III. | — | Story item |

---

## 11. Items

### Consumables

| Item | Effect | Cost |
|---|---|---|
| Bread Ration | Restore 25 HP | 12 |
| Herb Poultice | Restore 60 HP | 40 |
| Field Dressing | Restore 150 HP | 130 |
| Bottled Vim | Restore 20 MP | 90 |
| Emetic of Last Resort | Cure poison | 25 |
| Splint | Cure paralysis | 35 |
| Bell of Mild Alarm | Flee any non-boss battle | 60 |
| Return Voucher | Warp to last visited town | 100 |
| **Notarized Extension** | Auto-revive once at 50% HP. Death is a process, and processes can be appealed. | 900 |

Solo hero means death normally ends the run — the Notarized Extension is the safety valve
and should be introduced by Mudwick. Sell it cheap enough to be reachable.

### Key Items

| Item | Purpose |
|---|---|
| Guild Commission | Act I quest item. Openable from the menu; text changes per act. |
| Brass Key | Opens the Sump |
| Miner's Writ | Opens Bellhollow |
| Warden's Seal | Opens the Unfinished Tower |
| Crown Fragment I–IV | The plot |
| The Vex File | Readable. Contradictory dates. Foreshadows Act III. |

**The Vex File is the best foreshadowing device in the game.** Make it readable from the
menu, several pages long, and let the player notice the inconsistencies unprompted.

---

## 12. Spells

Solo hero learns everything on the level table below.

### Checkpoint D dependency — RESOLVED 2026-08-22

**MP pool chosen.** Use the MP column below directly. The FF1-style per-tier charge
variant was offered and declined, so the 2D charge array is not built; if a later
chapter wants to teach 2D arrays it needs a different vehicle.

| Spell | Lv | MP | Effect |
|---|---|---|---|
| Mend | 2 | 3 | Restore 35 HP |
| Ember | 3 | 4 | 25 fire damage |
| Dull | 5 | 4 | Enemy ATK −25%, 4 turns |
| Frost Nip | 6 | 6 | 40 ice damage |
| Adjourn | 7 | 7 | Sleep, 3 turns. Wears off if struck. |
| Purge | 8 | 5 | Cure all status |
| Stoneskin | 9 | 8 | Self DEF +40%, 5 turns |
| Greater Mend | 10 | 10 | Restore 110 HP |
| Jolt | 12 | 11 | 70 lightning damage |
| Hasten | 13 | 12 | Self AGI +50%, 5 turns |
| Immolate | 15 | 18 | 130 fire damage |
| Ward | 16 | 14 | Absorb next 80 damage |
| Sunder | 18 | 30 | 240 damage. The crown's own magic, turned outward. |

**Design note for Chapter 18:** implement spell effects as a data table of function
pointers, not a `switch`. This is the payoff for Chapter 14's dispatch-table lesson —
call it back explicitly.

---

## 13. Enemies

Roughly 5 per region, escalating. All names follow §8: mundane noun, wrong adjective.

### Grubbin Vale outskirts *(Lv 1–4)*

| Enemy | HP | ATK | DEF | AGI | XP | Gold |
|---|---|---|---|---|---|---|
| Disgruntled Slime | 14 | 5 | 1 | 3 | 3 | 5 |
| Turnip Blight | 18 | 6 | 2 | 4 | 4 | 6 |
| Field Rat | 12 | 7 | 1 | 8 | 4 | 4 |
| Committee of Bats | 22 | 6 | 2 | 11 | 6 | 9 |
| Hedge Wizard (Provisional) | 26 | 9 | 3 | 6 | 9 | 18 |

### Wetwood Barrow *(Lv 4–7)*

| Enemy | HP | ATK | DEF | AGI | XP | Gold |
|---|---|---|---|---|---|---|
| Barrow Moth | 30 | 11 | 4 | 12 | 12 | 14 |
| Grave-Damp | 44 | 13 | 7 | 5 | 16 | 20 |
| Unlicensed Skeleton | 38 | 15 | 6 | 9 | 18 | 22 |
| Mourner | 52 | 14 | 9 | 7 | 22 | 30 |

### The Sump *(Lv 8–12)*

| Enemy | HP | ATK | DEF | AGI | XP | Gold |
|---|---|---|---|---|---|---|
| Sump Leech | 60 | 20 | 8 | 14 | 34 | 40 |
| Bog Hulk | 105 | 26 | 16 | 6 | 55 | 65 |
| Marsh Lantern | 70 | 24 | 10 | 16 | 46 | 58 |
| Drowned Clerk | 88 | 28 | 13 | 11 | 60 | 80 |

### Bellhollow Mine *(Lv 12–15)*

| Enemy | HP | ATK | DEF | AGI | XP | Gold |
|---|---|---|---|---|---|---|
| Tunnel Grub | 130 | 33 | 20 | 8 | 95 | 110 |
| Feral Filing Cabinet | 160 | 36 | 28 | 5 | 130 | 160 |
| Deep Ringer | 115 | 40 | 17 | 19 | 120 | 140 |
| Pit Warden | 185 | 44 | 24 | 13 | 165 | 200 |

### The Unfinished Tower *(Lv 15–19)*

| Enemy | HP | ATK | DEF | AGI | XP | Gold |
|---|---|---|---|---|---|---|
| Half-Built Golem | 240 | 52 | 34 | 7 | 260 | 280 |
| Draft Revenant | 200 | 58 | 26 | 20 | 290 | 320 |
| Amanuensis | 180 | 55 | 30 | 23 | 310 | 350 |
| Sorcerer's Idea | 260 | 62 | 32 | 17 | 380 | 420 |

*Sorcerer's Idea* is a fragment of the belief that made Vex — pure foreshadowing. Give it
a battle description that unsettles rather than jokes.

---

## 14. Bosses

| Boss | Where | HP | ATK | DEF | AGI |
|---|---|---|---|---|---|
| **Bogwright** | Wetwood Barrow | 180 | 18 | 10 | 6 |
| **The Sump Auditor** | The Sump | 420 | 32 | 18 | 12 |
| **Bellhollow Tolling** | Bellhollow Mine | 700 | 46 | 26 | 15 |
| **The Unfinished Knight** | Unfinished Tower | 1,100 | 60 | 36 | 18 |
| **Vex** | The Crownless Court | 2,400 | 74 | 42 | 24 |

**Bogwright** — a mud golem of impeccable manners. Apologises before each attack.
Guarding the barrow because someone asked it to, a very long time ago, and never came
back to say stop. Teaches the boss-fight pattern; forgiving.

**The Sump Auditor** — a slime that has taken an interest in the player's inventory and
will be asking about several of these acquisitions. Steals gold. Drop it on defeat.

**Bellhollow Tolling** — a possessed bell. Attacks are sound: an area stun, a fear effect.
Introduces status-heavy combat. First fight where Purge matters.

**The Unfinished Knight** — a construct abandoned mid-fabrication, missing an arm and most
of a leg, still executing its final order with total commitment. Played straight. The
first fight that isn't funny. Vex takes the fragments immediately after.

**Vex** — three phases. Opens theatrical, quoting a villain speech he was never actually
given. Mid-fight he starts asking questions. Final phase he is barely holding shape.
He should get harder and quieter simultaneously. No jokes in this room.

---

## 15. Shops

| Town | Weapons | Armour | Items |
|---|---|---|---|
| **Grubbin Vale** | Bronze Shortsword | Padded Jerkin | Bread Ration, Emetic, Lucky Sock |
| **Mudwick** | Woodsman's Axe, Guild-Issue Longsword | Boiled Leather, Ring Mail | Herb Poultice, Bottled Vim, Bell of Mild Alarm, Notarized Extension, Ward-Charm |
| **Castle Hollis** | Silver Rapier, Sunsteel Blade¹ | Guild Plate, Sunsteel Harness¹ | Field Dressing, Return Voucher, Splint, Guild Signet, Ledger-Weight |

¹ Stocked only after Fragment III.

**Inns:** Grubbin Vale 6g, Mudwick 25g, Castle Hollis 90g. Full HP/MP restore and the
save point, Dragon Warrior style. Sell price is 50% of buy price throughout.

---

## 16. Sample Dialogue

Voice reference for Chapter 12. Match this register; don't quote verbatim beyond what's
useful.

**Bess Harrow (innkeeper, tutorial):**
> "Bed's six gold. Sleep fixes most things, in my experience. Not everything. Most."

**Old Sudd (elder, exposition, first pass):**
> "Four pieces. Scattered north, south, and — the other two directions. I had it written
> down."

**Old Sudd (second pass, contradicting himself):**
> "Three pieces, was it? No. Four. There were definitely four, because I remember thinking
> it was an inconvenient number."

**Deputy Registrar Ammel (the commission):**
> "The Guild is prepared to offer forty gold up front and a completion bonus that I'd
> rather not describe as generous. You should know that eleven others declined. You should
> also know the wards have four months, and that's the optimistic figure."

**Grubbin Vale villager:**
> "Something got into the turnips. Not a fox. Foxes don't do that to a fence."

**Mudwick shopkeeper:**
> "Everything's twice what it was. That's not me gouging. That's the roads."

**Perrin Culch (first meeting — plant this):**
> "Sorry — is this about the vault inventory? It's not ready. It's been eleven years and
> it's not ready."

**The Vex File (readable key item):**
> "Incident recorded 3rd of Harrowmass. Sorcerer identified as VEX. Description follows:
> tall. Further description was not obtained. The reporting clerk was, per his own
> account, elsewhere at the time."

**Bogwright (boss, on engagement):**
> "I would rather not. But I was asked, and nobody has since unasked me. My apologies for
> what follows."

**Vex (final, phase three):**
> "You've been to the barrow. The mine. The tower. Tell me what I did there. I have been
> told my whole life what I did there and I cannot remember any of it. Tell me."

---

## 17. Still Open

Do not decide these here. Ask at the specified checkpoints.

- **Checkpoint A** — renderer (ncurses vs SDL2), pacing, prior C experience.
- ~~**Checkpoint C**~~ — ✅ Answered 2026-08-22. Speed-sorted by AGI and the §9.2
  damage formula adopted as written. Two deviations: encounters gain an **off
  toggle**, and difficulty is **tunable via a config file** (`encounters`,
  `enemy_damage`, `xp_rate`, `gold_loss_on_death`). See `PROJECT_CHOICES.md`.
- ~~**Checkpoint D**~~ — ✅ Answered 2026-08-22. MP pool (not charges); solo hero learns
  every spell on schedule; level and spell tables move to data files in ch21.
- **Checkpoint E** — save format.
- **Checkpoint F** — Vex's fate (self-unmaking vs. put down), which optional dungeons
  ship, stretch goals, packaging.
