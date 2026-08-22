# Learn C by Building a JRPG

An interactive, incremental tutorial that teaches you C — the real language,
warts included — by having you build a genuinely playable JRPG in the spirit
of *Final Fantasy I* / *Dragon Warrior*: overworld, towns, dungeons, NPCs,
turn-based combat, inventory, magic, shops, and save/load, ending in a final
boss and credits.

This is a course, not a finished game. Every chapter teaches a C concept,
then has you apply it to grow the game by one visible step. Every chapter
ships a working, compiling snapshot of the project so far under `code/chNN/`
— a safety net, not the lesson itself.

Full specification: [`DESIGN.md`](DESIGN.md). Your recorded decisions, and
why they matter to later chapters: [`PROJECT_CHOICES.md`](PROJECT_CHOICES.md).

## How to use this course

1. Read the chapter in `chapters/`.
2. Work through it in your own project directory, following along.
3. If you get stuck, `code/chNN/` has the complete, correct end state for
   that chapter — copy it and keep going.
4. At a checkpoint (marked ⚑ below), you'll be asked to make a real design
   decision about your game. Answer honestly; later chapters build on it.

## Environment

Developed and played entirely inside **WSL2 running Arch Linux**, in the
Linux filesystem (never under `/mnt/c/...`). Chapter 0 walks through setup
from a bare WSL Arch install. See
[`appendices/a-wsl-troubleshooting.md`](appendices/a-wsl-troubleshooting.md)
for WSL-specific gotchas.

## Status

Chapters 0–17 written, each with a verified `code/chNN/` snapshot that
compiles warning-free (and, from Chapter 10 on, runs clean under
AddressSanitizer/UBSan). From Chapter 15 on there is also a `make test`
suite -- currently 93,460 assertions covering combat maths, levelling,
battle termination, inventory growth, and config parsing.

Checkpoints A through D are answered — terminal + ncurses; the world of
*Some Assembly Required*, where a solo hero named Wick recovers four
fragments of a shattered crown for an apologetic civil service; and
AGI-sorted turn order with an FF1-style damage formula, encounters you can
switch off, difficulty tunable from a config file, and magic paid for from
a single MP pool. Checkpoint E (Save Format) is next, before Chapter 20.
See `PROJECT_CHOICES.md` for the full ledger and `CONTENT.md` for the
content bible.

## Chapter Index

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
| 23 | **The Ending** ⚑ | packaging, `make install`, final polish | Boss, ending, credits, shippable |

⚑ = chapter preceded by a checkpoint (a required stop to ask you a design question).

## Appendices

- `a-wsl-troubleshooting.md` — WSL/Arch-specific gotchas collected in one place
- `b-c-pitfalls.md` — common C mistakes and how to spot them
- `c-gdb-and-sanitizers.md` — debugging reference
- `d-glossary.md` — every term defined on first use, indexed here too
- `e-stretch-goals.md` — optional extensions offered at Checkpoint F
