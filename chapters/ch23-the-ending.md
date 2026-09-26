# Chapter 23: The Ending

## Where we are

Twenty-two chapters in, the game is solid: no leaks, no undefined
behaviour, a test suite that runs three hundred thousand assertions, and a
fuzzer that throws four thousand malformed files at the parsers without
finding a scratch.

What it does not have is a point. There are three places to walk, nothing
to recover, nobody to face, and no way to finish. A player can wander
Grubbin Vale until they are bored, which is not the same as playing a game.

This chapter ends it. Three new locations, three bosses, four crown
fragments, and the last conversation Wick ever has.

## The problem

An ending is not one feature, it is five, and they have to arrive together:

- **Somewhere to go.** Mudwick, The Sump, and The Crownless Court, chosen
  at Checkpoint F over the eight-location version.
- **A reason to go in order.** Nothing currently stops a level 1 hero
  walking into the last room.
- **Something at the end of each place.** A boss is not just a big enemy;
  it should change as it loses.
- **The fragments.** *A Quest in Four Pieces* has, to date, zero pieces.
- **A last screen.** Not a `printf` on the way out — an actual ending.

There is also a debt to pay. Several things in this game have been quietly
broken for chapters, and an ending is exactly the thing that trips over
them. We will find three of those, and one of them would have made the
final boss impossible to beat.

## C concept spotlight

### Functions that take any number of arguments

You have called `printf` several hundred times without asking the obvious
question: how does a function accept a different number of arguments each
time? Every function you have written takes a fixed list. `printf` clearly
does not.

The answer is **variadic functions**, and the machinery is in `<stdarg.h>`.
Here is a complete program — save it as `varargs.c` and compile it on its
own:

```c
#include <stdio.h>
#include <stdarg.h>

/* The ... means "and then some number of further arguments". At least
   one named parameter must come first -- here, count -- because
   va_start needs something to start from. */
static int sum(int count, ...)
{
    va_list args;          /* the cursor over the extra arguments */
    int total = 0;

    va_start(args, count); /* begin, just after the last named one */

    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);   /* read one, advance the cursor */
    }

    va_end(args);          /* required; on some platforms it matters */

    return total;
}

int main(void)
{
    printf("%d\n", sum(3, 10, 20, 30));
    printf("%d\n", sum(5, 1, 1, 1, 1, 1));
    return 0;
}
```

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -g varargs.c -o varargs
./varargs
```

```
60
5
```

Four pieces, and they always appear together:

| | |
|---|---|
| `va_list` | the cursor over the unnamed arguments |
| `va_start(args, last_named)` | position it after the last named parameter |
| `va_arg(args, TYPE)` | read the next argument **as TYPE**, and advance |
| `va_end(args)` | finish with it |

### Why the format string exists

Look hard at `va_arg(args, int)`. You told it the type. It did not work
the type out — it *cannot*. Nothing about the extra arguments is recorded
anywhere at runtime: not how many there are, not what types they were.
`va_start` gives you a cursor into a pile of bytes and trusts you
completely.

That is the whole reason `printf` takes a format string. `"%d %s"` is not
decoration; it is the *only* record of what was passed. When you write:

```c
printf("%d\n", "hello");    /* a pointer, read as an int */
```

...`printf` reads a pointer's bytes as an `int` because you said `%d`, and
prints whatever that happens to be. That is undefined behaviour, and it is
the classic C security hole: a variadic function is a hole in the type
system, and the format string is the only patch over it.

Which is why compilers cheat on your behalf:

```c
__attribute__((format(printf, 2, 3)))
static void log_linef(Battle *battle, const char *fmt, ...);
```

That tells gcc "argument 2 is a printf format string, and the variadic
arguments start at 3" — so it type-checks the call sites exactly as it does
for `printf` itself. It is a GNU extension rather than standard C, but gcc
and clang both support it, and the alternative is no checking at all. Use
it on every variadic function you write that takes a format.

### Forwarding: the `v` twins

Our wrapper has a problem. It has a `va_list`, but `snprintf` wants loose
arguments — you cannot expand a `va_list` back out.

The standard library solves this by providing a second version of every
formatted-output function that takes a `va_list` instead: `vprintf`,
`vfprintf`, `vsnprintf`. They exist for exactly this purpose — so that you
can write wrappers.

```c
static void log_linef(Battle *battle, const char *fmt, ...)
{
    char line[BATTLE_LOG_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(line, sizeof line, fmt, args);
    va_end(args);

    log_line(battle, line);
}
```

The rule of thumb: if you are writing a function that formats, take
`const char *fmt, ...`, and immediately hand off to the `v` version.

### Why we want it

Because `battle.c` is full of this:

```c
char line[BATTLE_LOG_LEN];
snprintf(line, sizeof line, "The %s hits you for %d.", enemy->type->name, damage);
log_line(battle, line);
```

Eight times, each with its own scratch buffer. With `log_linef` every one
of them collapses to its middle line:

```c
log_linef(battle, "The %s hits you for %d.", enemy->type->name, damage);
```

That is the whole refactor. Do it now, before the bosses arrive and add
more.

## Apply it: bosses

### A boss is an enemy with stages

Add this to `battle.h`, above the `Battle` struct:

```c
#define BOSS_PHASE_COUNT 3

typedef enum {
    BOSS_BOGWRIGHT,
    BOSS_SUMP_AUDITOR,
    BOSS_VEX,
    BOSS_COUNT
} BossId;

typedef struct {
    EnemyType type;
    const char *phase_lines[BOSS_PHASE_COUNT];
    int reward_item;      /* key item granted on victory */
} BossType;

const BossType *battle_boss(BossId id);
```

A `BossType` **contains** an `EnemyType` rather than pointing at one, so a
boss is an enemy plus extras. Everything in `battle.c` that already works
on `const EnemyType *` keeps working, pointed at `&boss->type`.

And two new fields in `Battle`:

```c
int boss_id;          /* BossId, or -1 for an ordinary encounter */
int boss_phase;       /* last phase announced, so it announces once */
```

### Derive state; don't store it

The phase could be a variable that combat code updates. It should not be.
Store it and you have two facts that can disagree: the HP bar the player is
looking at, and a number saying which phase you are in. Derive it and
disagreement is impossible.

```c
int battle_boss_phase(const Battle *battle)
{
    if (battle->boss_id < 0 || battle->enemy_count < 1) {
        return 0;
    }

    const Enemy *enemy = &battle->enemies[0];
    int max_hp = enemy->type->max_hp;
    if (max_hp <= 0) {
        return 0;
    }

    /* Multiply rather than divide: (hp * 3) / max_hp keeps the whole
       calculation in integers without losing the fraction to rounding. */
    int thirds = (enemy->hp * 3) / max_hp;

    if (thirds >= 2) return 0;
    if (thirds >= 1) return 1;
    return 2;
}
```

Note the arithmetic. The tempting version is `hp / max_hp * 3`, which in
integer arithmetic is `0 * 3` for every value below full — integer division
throws the fraction away *before* the multiply. Multiplying first keeps it.

`boss_phase` is still stored, but it holds something much smaller: the
highest phase already *announced*, so the boss speaks once per stage rather
than every round.

### The one thing memset cannot do

`battle_begin` starts with `memset(battle, 0, sizeof *battle)`, and that is
where a trap opens:

```c
memset(battle, 0, sizeof *battle);

/* memset zeroes boss_id, and zero is a valid BossId -- so say "not a
   boss" explicitly, before any early return can leave it saying
   "Bogwright". */
battle->boss_id = -1;
```

Zeroing a struct is not the same as emptying it. It sets every field to
zero, and zero *means something* for an enum: `BOSS_BOGWRIGHT` is 0, and
`BATTLE_ONGOING` is 0. A zeroed `Battle` claims a Bogwright fight is under
way. Whenever 0 is a legal value of a field, zeroing cannot express
"absent", and you must say so in another line.

This exact trap catches us twice more before the chapter is out.

## Apply it: the world

### Three new places

Three maps go in `assets/maps/` — `mudwick.map`, `the_sump.map`, and
`the_crownless_court.map` — and three rows go in `world.c`'s `area_table`,
with matching entries in the `AreaId` enum, the enemy `rosters` in
`battle.c`, and the `shops` table.

That list is the point. Adding a third of the game's world touches five
tables and no logic at all. Chapters 13 and 21 built it that way
deliberately; this is where the interest is paid.

Raise the warp cap first, in `world.h` — Castle Hollis now has three exits:

```c
#define WORLD_MAX_WARPS_PER_AREA 3
```

### Progress without a quest system

The story has an order: barrow, then Mudwick, then the Sump, then the
court. Enforcing it usually means quest flags — a pile of booleans that
must be defined, updated, saved, loaded, and validated.

We already have somewhere better to put it. The player's *inventory* is
saved and loaded and validated, and the fragments are already in it. So a
gate asks the pack:

```c
typedef struct {
    int x;
    int y;
    AreaId destination;
    int destination_x;
    int destination_y;

    /* Progress gating. A warp with requires_item set to an item id only
       opens once that key item is carried; -1 means always open. */
    int requires_item;
    const char *locked_speech;
} Warp;
```

```c
{ .x = 28, .y = 15, .destination = AREA_THE_CROWNLESS_COURT,
  .destination_x = 10, .destination_y = 11,
  .requires_item = ITEM_FRAGMENT_II,
  .locked_speech =
    "The door to the old court is sealed. Whatever opens it, "
    "you are not carrying enough of it yet." }
```

Zero new save fields. Zero new validation. The fragments *are* the progress
record, and they were already being written to disk in Chapter 20.

The same trick retires the boss respawn problem. A boss whose reward is
already in the pack has been beaten, so there is nothing to check and
nothing to store:

```c
if (boss != NULL &&
    inventory_count_of(&game->inventory, (ItemId)boss->reward_item) == 0) {
    battle_begin_boss(&game->battle, (BossId)trigger->boss_id);
    game->mode = MODE_BATTLE;
    return;
}
```

**Design worth stealing:** before adding state, look for state you already
keep that implies the answer. It cannot get out of step with itself.

## Three bugs an ending finds

Here is where the chapter stops being about new code.

### Bug 1: a button that was never wired up

The battle screen has said this since Chapter 18:

```
Enter attack, m magic, w/s target, f flee
```

Press `m` in a fight and nothing happens. `battle_handle` never had a case
for it. The whole spell path was built — `spells_cast_selected` targets the
selected enemy, spends the hero's action, calls `battle_enemy_turn`, and
returns to `MODE_BATTLE` — and nothing could reach it.

```c
/* The battle screen has said "m magic" since Chapter 18, and until now
   nothing here listened for it. Everything the spell menu needs to work
   mid-fight already existed; only the door was missing. */
case INPUT_MENU:
    game->spell_index = 0;
    game->mode = MODE_SPELLS;
    return;
```

Nothing caught this. Not the compiler — both sides compiled. Not the tests
— they call combat functions directly and never press a key. Not the
Chapter 22 monkey test — it pressed `m` a hundred times and correctly
reported that nothing crashed. *Nothing crashing is not the same as
something happening.*

### Bug 2: winning the wrong way paid nothing

Worse, and only visible once magic worked at all. `spells_cast_selected`
ended like this:

```c
game->dialog_return = (in_battle && game->battle.outcome == BATTLE_ONGOING)
                    ? MODE_BATTLE : MODE_EXPLORE;
```

Kill the last enemy with a spell and the battle is over, so `outcome` is no
longer `BATTLE_ONGOING`, so the player is returned to the map — **without
`battle_finish` ever running.** No XP. No gold. No key item. And for the
final boss, no ending: kill Vex with a spell and the game would hand you
back to an empty throne room forever.

```c
/* If the spell -- or the blow that answered it -- ended the fight, hand
   off to the same function an ordinary victory uses. Without this,
   killing the last enemy with magic awarded no XP, no gold, and no key
   item, and the final boss never triggered the ending.
   One exit path, used by every way a battle can end. */
if (in_battle && game->battle.outcome != BATTLE_ONGOING) {
    battle_finish(game);
    return;
}
```

The lesson is in the comment. A battle could end in two places, and only
one of them settled up. When something can finish in more than one way,
funnel every way through a single exit.

### Bug 3: an uninitialised struct, ten chapters late

Fixing bug 1 meant `spells_handle` now asks whether a battle is running, in
order to know where the cancel key should return to. That question is asked
before the player's first fight — and `game->battle` had never been
initialised. Valgrind, on the full game:

```
==289259== ERROR SUMMARY: 10 errors from 10 contexts (suppressed: 3 from 3)
```

```
==289259== Conditional jump or move depends on uninitialised value(s)
==289259==    at 0x40033AA: battle_draw (game.c:359)
==289259==    by 0x4005558: game_draw (game.c:1203)
==289259==    by 0x4002593: main (main.c:66)
```

Ten, in a game that finished Chapter 22 on zero. And the fix is the trap
from earlier, for the third time:

```c
/* There is no battle yet, and several places ask whether one is in
   progress before the first encounter ever happens.

   Note that zeroing alone would be wrong: BATTLE_ONGOING is 0, so a
   memset would claim a fight is under way. Say "finished" out loud,
   exactly as battle_begin does for an area with no roster. */
memset(&game->battle, 0, sizeof game->battle);
game->battle.outcome = BATTLE_WON;
game->battle.boss_id = -1;
```

```
==289688== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 3 from 3)
```

Chapter 22 was not a chapter you do once. It is a habit. New code gets the
same treatment as old code, and the tools have to be re-run every time — the
game was clean when Chapter 22 ended and dirty again three hundred lines
later.

## Apply it: the ending

Six passages, in a table, because words are content:

```c
static const char *const ending_pages[] = {
    "The four pieces sit together on the flagstones, and do not look "
    "like a crown. They look like four pieces.",
    /* ... */
};
```

The first version of `ending_draw` used `dialog_wrap`, and lost half the
ending. `dialog_wrap` renders **the first page only** — everything past
four lines is silently dropped, which is precisely the bug Chapter 12's
paging exists to prevent and which bit this project once already.

So the ending uses the paging that is already there:

```c
static void ending_handle(Game *game, InputEvent event)
{
    if (event == INPUT_NONE) {
        return;
    }

    /* Finish the current passage before moving to the next one. */
    if (dialog_advance(&game->dialog)) {
        return;
    }

    if (game->ending_page + 1 < ENDING_PAGE_COUNT) {
        ending_show_page(game, game->ending_page + 1);
        return;
    }

    game->running = 0;
}
```

Two nested "next" steps: next screen within a passage, then next passage.
Fifteen screens in total, and nothing lost.

### Cutting the coin flip

`main.c` still called `duel_run()` — Chapter 3's coin-flip duel — *after*
the game loop. Which meant the ending would fade out and a stranger would
offer to flip a coin.

It goes, along with `duel.c` and `duel.h` and its line in the `Makefile`.
It did its job in Chapter 3; shipping it in the finished game would be
keeping the scaffolding up after the building is done.

## Balance: measuring instead of guessing

The content bible's boss stats were transcribed in good faith and labelled
unplaytested. They were also, it turns out, unwinnable.

Rather than play the game forty times, write a simulation: link the real
`battle.c`, `party.c` and `combat_math.c` against a `main` that runs four
hundred fights per level and counts. It is fifty lines, it uses the actual
combat code rather than a model of it, and it takes two seconds.

The first run said this, for a hero at the level cap with the best gear in
the game:

```
--- Vex (HP 1400 ATK 52 DEF 30) ---
  Lv20  win   0.0%   avg 13 rounds
```

Not "hard". **Zero.** At level 15 the hero deals about 37 damage a round
and needs 38 rounds; Vex deals about 32 and needs 5. No amount of skill
closes that.

After retuning Vex to 600 HP, 34 attack, 22 defence:

```
--- Bogwright (HP 180 ATK 18 DEF 10) ---
  Lv6   win  49.0%   avg 16 rounds
  Lv7   win  87.8%   avg 15 rounds
--- Sump Auditor (HP 420 ATK 32 DEF 18) ---
  Lv11  win  25.2%   avg 22 rounds
  Lv12  win  78.5%   avg 19 rounds
--- Vex (HP 600 ATK 34 DEF 22) ---
  Lv13  win  28.0%   avg 28 rounds
  Lv14  win  72.0%   avg 29 rounds
  Lv15  win  87.8%   avg 25 rounds
```

Each boss has a level where it flips from unlikely to likely, and they are
spaced across the game: 6–7, 11–12, 14–15. A player who is under-levelled
loses and knows why; one who has explored wins with something to spare.

Two honest caveats. The simulation heals with `Mend` and never uses items,
so it is pessimistic. And these are still *numbers*, not playtesting —
they tell you a fight is winnable, not whether it is any fun.

## Compile and run

```bash
make clean
make
./game
```

Walk south out of Grubbin Vale to Castle Hollis, then try the south door:

```
The south road is closed to unaccredited persons. The Guild
suggests returning with something to show.
```

Go to Wetwood Barrow instead and walk into the far chamber:

```
Bogwright.
The Bogwright apologises, and swings.
```

Take it to two thirds and it says the second of its three lines; take it
below a third and it says the last one. Win, and:

```
It stops. Among what it leaves behind is a piece of worked
gold, heavier than it has any right to be.
```

That door in Castle Hollis is open now.

```bash
make test
```

```
303274 checks, 0 failed
```

```bash
make valgrind
```

```
==289887== All heap blocks were freed -- no leaks are possible
==289887== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

And the whole game, played to the end under memcheck:

```
==289688==    definitely lost: 0 bytes in 0 blocks
==289688== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 3 from 3)
```

## What just happened

Three ideas did most of the work in this chapter, and none of them is about
bosses.

**Derived state cannot disagree with itself.** The boss phase is computed
from HP and the "have you beaten this" flag is computed from the inventory.
Neither is stored, so neither can drift, and neither needed a line in the
save file.

**Zero is a value.** `memset` does not empty a struct, it fills it with
zeroes, and for an enum whose first member is `BATTLE_ONGOING` or
`BOSS_BOGWRIGHT`, zero is a confident lie. Three separate bugs this chapter
were that one idea.

**One exit path.** A battle could end in two places and only one settled
up. The version that pays XP is the version that grants key items is the
version that starts the ending — because they are the same function.

And a fourth, which is really about process: the compiler, the tests, the
sanitizers and the fuzzer between them said this code was fine, while a
button on the screen did nothing at all. Every tool has a shape of bug it
looks for. None of them plays your game.

## Common errors

**`va_start` with the wrong parameter:**

```c
static void log_linef(Battle *b, const char *fmt, ...)
{
    va_list args;
    va_start(args, b);       /* wrong: must be the LAST named parameter */
```

```
e1.c:7:5: warning: second parameter of ‘va_start’ not last named
argument [-Wvarargs]
```

The arguments come out shifted by one — usually garbage, sometimes a crash.

**Forgetting the format attribute, then getting the type wrong:**

```c
log_linef(battle, "The %s falls.", damage);     /* an int, read as char* */
```

Without `__attribute__((format(printf, 2, 3)))` this compiles silently and
dereferences `damage` as a pointer at runtime. With it:

```
warning: format ‘%s’ expects argument of type ‘char *’, but argument 3
has type ‘int’ [-Wformat=]
```

**Using `snprintf` where you need `vsnprintf`:**

```c
snprintf(line, sizeof line, fmt, args);   /* args is a va_list, not the values */
```

A `va_list` cannot be expanded back into loose arguments. Any function that
receives one must pass it to a `v`-prefixed function.

**Trusting `memset` to mean "none":**

```c
memset(battle, 0, sizeof *battle);
/* battle->boss_id is now 0, which is BOSS_BOGWRIGHT */
```

No warning, no crash — just a phantom Bogwright. If zero is a legal value,
zeroing cannot express absence.

**Integer division before multiplication:**

```c
int thirds = enemy->hp / enemy->type->max_hp * 3;    /* 0, or 3 at full HP */
```

`hp / max_hp` is 0 for every value below full, so the whole expression is
0 until the boss is untouched, when it jumps straight to 3. Multiply
first: `(hp * 3) / max_hp`.

## Exercises

> **Practice drills:** `code/ch23/practice/01-format-contract/` (`make &&
> make check`, then `make bad`) plus micros `01_format_attr`,
> `02_phase_index`, `03_sim_potion_gate`. Do **not** delete the
> `__attribute__((format(...)))` line from production `log_linef` to see
> what happens — that comparison lives in the drill.

1. *Format contract (practice).* Complete `01-format-contract`. Run `make bad`
   and read both compiler outputs (with vs without the attribute on an
   intentional `%s`/int mismatch). What does keeping the attribute buy you
   before runtime?
2. Items cannot be used in battle — the battle screen never offered them,
   so unlike magic this is a missing feature rather than a broken promise.
   Wire `MODE_ITEMS` in the same way magic now works, and update the help
   line. Which of the three bugs in this chapter does your change risk
   reintroducing?
3. Add a fourth phase line to one boss and change `BOSS_PHASE_COUNT` to 4.
   What else has to change, and what happens if you change the constant
   but not `battle_boss_phase`? (Rehearse clamp/index ideas in
   `02_phase_index` if you want.)
4. *Open-ended:* Vex was rebalanced with a simulation that heals but never
   uses items. Extend it to drink a Herb Poultice when out of MP, and see
   what that does to the win rates. Then decide: should the boss be tuned
   against a player who plays well, or one who plays adequately? Write
   down which you chose and why — it is the single most consequential
   number in the game, and there is no correct answer. (`03_sim_potion_gate`
   is a tiny rehearsal of the gate.)

<details>
<summary>Solutions</summary>

1. See `01-format-contract/SOLUTION.md`. With the attribute, gcc reports
   `format '%s' expects argument of type 'char *', but argument … has type
   'int'`. Without it, the call can compile clean and `vsnprintf` treats the
   integer as an address — typically a segfault, occasionally garbage, and
   on a very unlucky day nothing visible. Keep the attribute on shipping
   wrappers.
2. Add handling for a new key, set `game->item_index = 0`, switch to
   `MODE_ITEMS`, and make the items screen return to `MODE_BATTLE` when a
   fight is running. The risk is **bug 2**: any new way to end a fight must
   go through the one `battle_finish` exit.
3. `BossType.phase_lines` sizes itself from the constant, so it grows
   automatically — but `battle_boss_phase` still returns only 0, 1 or 2,
   so the fourth line is never spoken. The test that checks every phase
   line is non-NULL fails on purpose. Constant and thresholds must change
   together.
4. Adding items typically pushes the level-14 win rate well above 80%.
   Tuning against a well-played run makes the boss harder for everyone
   else; tuning against an adequate one makes it trivial for anyone who
   prepares. Most games tune for the middle and let difficulty settings
   cover the ends — which this game already has in `assets/config.txt`'s
   `enemy_damage`.

</details>

## Next up

The game is finished. It has an overworld, two towns, a capital, two
dungeons, a final court, NPCs, random encounters you can switch off,
turn-based combat, levelling, inventory, magic, equipment, shops,
save and load, three bosses, four crown fragments, and an ending.

It also only runs on the machine it was built on, from the directory it was
built in. **Chapter 24** fixes that: static and dynamic linking, what
`-lncursesw` actually attaches to your binary, `make install`, and how to
hand somebody a copy of this that runs.
