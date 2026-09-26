# Chapter 16: The Battle Loop

## Where we are

Chapter 15 gave you damage numbers you can trust and a generator you can
replay, both covered by 87,000 test assertions. Now you spend them.
This chapter adds enemies, turn order, a `MODE_BATTLE` that slots into the
Chapter 14 dispatch table without disturbing anything, random encounters on
a step counter — with the off switch you asked for at Checkpoint C — and
experience points that make the numbers grow.

It also contains the first time in this course that **measuring the game
proved a design wrong**, which turns out to be the most valuable part.

## The problem

A battle is a state machine inside a state machine. The game is in battle
mode; within that, the battle itself has a turn order, a target cursor, a
running log, and an outcome that has to be checked after every action.
Getting the *structure* right matters more than the maths here — a battle
that can't end, or that lets you attack a corpse, or that awards XP twice,
will not crash. It will just be wrong.

## C concept spotlight

### Arrays of structs, revisited — with liveness

Chapter 13 used arrays of structs for static world data. Battle data
changes:

```c
typedef struct {
    const EnemyType *type;   /* shared, never modified */
    int hp;                  /* this individual's remaining health */
    int alive;
} Enemy;
```

The split matters. `EnemyType` holds what's true of *every* Disgruntled
Slime — name, max HP, attack, XP value — and lives in a `static const`
table. `Enemy` holds what's true of *this* slime, right now. Three slimes
in one fight are three `Enemy` structs pointing at one shared `EnemyType`.

That's a **flyweight**: the per-instance data is small, the shared data is
stored once. It also means the roster is immutable and can be `const`,
so a bug can't accidentally lower every future slime's HP.

The `alive` flag rather than deleting from the array is deliberate. Removing
an element means shuffling everything after it down, which invalidates the
target index the player is holding. Marking it dead keeps every index
stable for the whole battle — a "tombstone", and the standard answer when
positions have meaning.

### State within state

`Battle` is a complete state machine that knows nothing about `Game`:

```c
typedef struct {
    Enemy enemies[BATTLE_MAX_ENEMIES];
    int enemy_count;
    int target;
    BattleOutcome outcome;
    int xp_reward;
    int gold_reward;
    char log[BATTLE_LOG_LINES][BATTLE_LOG_LEN];
    int log_count;
} Battle;
```

`BattleOutcome` is the whole reason this is tractable:

```c
typedef enum {
    BATTLE_ONGOING,
    BATTLE_WON,
    BATTLE_LOST,
    BATTLE_FLED
} BattleOutcome;
```

Every function that changes the battle sets `outcome`, and exactly one
place checks it. Without an explicit outcome you end up asking "are all
enemies dead? is the hero dead? did they flee?" in four different places
and getting a different answer in one of them.

### A scrolling log, without allocation

The battle needs to show the last few things that happened:

```c
static void log_line(Battle *battle, const char *text)
{
    if (battle->log_count >= BATTLE_LOG_LINES) {
        /* Scroll: drop the oldest line and shuffle the rest up. */
        for (int i = 1; i < BATTLE_LOG_LINES; i++) {
            memcpy(battle->log[i - 1], battle->log[i], BATTLE_LOG_LEN);
        }
        battle->log_count = BATTLE_LOG_LINES - 1;
    }

    snprintf(battle->log[battle->log_count], BATTLE_LOG_LEN, "%s", text);
    battle->log_count++;
}
```

Fixed-size, inside the struct, no `malloc` anywhere. When it's full, every
line shifts up one and the oldest is overwritten. `snprintf` with
`BATTLE_LOG_LEN` guarantees termination and prevents overflow no matter
what enemy name gets formatted in — the Chapter 12 discipline applied
without thinking about it.

That `memcpy` moves whole rows of a 2D array. `battle->log[i]` is a
`char[BATTLE_LOG_LEN]`, so copying `BATTLE_LOG_LEN` bytes from one row to
another is exactly one line's worth. This is the flat-memory model from
Chapter 5 being useful rather than dangerous.

### Turn order without a sort

Checkpoint C chose AGI-sorted turn order, ties to the hero. With a solo
hero you don't need to sort anything — you need to know which enemies act
*before* the hero and which act *after*:

```c
/* Enemies faster than the hero strike first. */
for (int i = 0; i < battle->enemy_count; i++) {
    const Enemy *enemy = &battle->enemies[i];
    if (enemy->alive && enemy->type->agility > player->agility) {
        enemy_attacks(battle, player, rng, enemy);
        if (player->hp <= 0) {
            battle->outcome = BATTLE_LOST;
            return;
        }
    }
}

hero_attacks(battle, player, rng);

if (battle_living_enemies(battle) == 0) {
    battle->outcome = BATTLE_WON;
    return;
}

/* Then everyone the hero outpaced (ties included -- hero wins ties). */
for (int i = 0; i < battle->enemy_count; i++) {
    const Enemy *enemy = &battle->enemies[i];
    if (enemy->alive && enemy->type->agility <= player->agility) {
        /* ... */
    }
}
```

Two passes with a comparison is a sort — of a list with one interesting
element in it. The strict `>` in the first pass and `<=` in the second is
where "ties go to the hero" actually lives: an enemy with equal agility
falls into the second group, so the hero swings first. One character
carries a design decision, which is worth a comment and a test.

Note the `if (player->hp <= 0)` check *inside* the loop, not after it.
Without it, a dead hero keeps taking hits from the remaining enemies in
the same round — mechanically harmless but nonsense to read in the log.


### Fight-scoped state vs hero-scoped state

A flag like "defending this round" belongs on `Battle`, not on `Player`.
Hero fields survive the fight and every exit path; battle fields die with
the encounter. Clear round-scoped flags at a single, boring place (end of
`battle_round`) — too early and defend never works; too late and it lasts
forever. Practice `01_defend_flag` engrains the lifetime before you touch
`battle.c`.


## Apply it

### Levels, from a table

`party.h` / `party.c` hold the progression, transcribed from the content
bible:

```c
typedef struct {
    int hp;
    int mp;
    int attack;
    int defense;
    int agility;
    int xp_to_next;   /* XP needed to leave this level; 0 at the cap */
} LevelRow;

static const LevelRow level_table[PARTY_MAX_LEVEL] = {
    /*  HP   MP  ATK  DEF  AGI   XP to next */
    {   20,   0,   5,   3,   5,      8 },   /* level 1  */
    {   26,   2,   6,   4,   6,     18 },
    /* ... through level 20 ... */
    {  240,  85,  52,  41,  27,      0 }    /* level 20, the cap */
};
```

**These numbers are unplaytested.** The content bible says so explicitly,
and this chapter is about to prove it.

`party_apply_level` writes the row's stats onto the hero, and
`party_gain_xp` levels up as many times as the XP allows:

```c
int party_gain_xp(Player *p, int xp)
{
    if (xp <= 0) {
        return 0;
    }

    p->xp += xp;

    int levels_gained = 0;

    while (p->level < PARTY_MAX_LEVEL) {
        const LevelRow *row = party_level_row(p->level);

        if (row->xp_to_next <= 0 || p->xp < row->xp_to_next) {
            break;
        }

        p->xp -= row->xp_to_next;
        p->level++;
        levels_gained++;

        party_apply_level(p);
        p->hp = p->max_hp;
        p->mp = p->max_mp;
    }

    return levels_gained;
}
```

The `while` handles a single reward crossing several levels — a real case
when you return to easy areas late. Two guards keep it from spinning
forever: `p->level < PARTY_MAX_LEVEL`, and `row->xp_to_next <= 0`. Either
alone would do; both is cheap insurance against a table typo turning into
a hang.

Note `entity_create_player` no longer hardcodes stats — it sets level 1 and
calls `party_apply_level`, so stats are defined in exactly one place.

### Enemies, as another lookup table

```c
static const EnemyType grubbin_vale_enemies[] = {
    { "Disgruntled Slime",  14,  5, 1,  3, 3,  5 },
    { "Turnip Blight",      18,  6, 2,  4, 4,  6 },
    { "Field Rat",          12,  7, 1,  8, 4,  4 },
    { "Committee of Bats",  22,  6, 2, 11, 6,  9 }
};

typedef struct {
    const EnemyType *types;
    int count;
    int max_group;
} Roster;

static const Roster rosters[] = {
    { grubbin_vale_enemies, /* count */, 3 },
    { wetwood_barrow_enemies, /* count */, 2 },
    /* Castle Hollis is a safe hub: no roster, no encounters. */
    { NULL, 0, 0 }
};
```

Indexed the same way `AreaId` is — Chapter 13's technique, reused without
modification. Castle Hollis having a `NULL` roster is how "safe area" is
expressed: data, not a special case in code.

### Encounters, with the Checkpoint C off switch

```c
#define ENCOUNTER_MIN_STEPS 12
#define ENCOUNTER_PERCENT   22

static void maybe_start_encounter(Game *game)
{
    if (!game->encounters_enabled) {
        return;
    }

    game->steps_since_encounter++;

    if (game->steps_since_encounter < ENCOUNTER_MIN_STEPS) {
        return;
    }

    if (rng_range(&game->rng, 0, 99) >= ENCOUNTER_PERCENT) {
        return;
    }

    battle_begin(&game->battle, &game->rng, (int)game->world->current,
                 game->player.level);

    /* A safe area has no roster, so battle_begin hands back an
       already-won battle; don't switch modes for that. */
    if (game->battle.outcome != BATTLE_ONGOING) {
        return;
    }

    game->steps_since_encounter = 0;
    game->mode = MODE_BATTLE;
}
```

The minimum-step gap is what stops two fights in a row, which pure
randomness would produce regularly and players would rightly hate. The
early return on `!encounters_enabled` is your Checkpoint C toggle, exposed
in the Chapter 14 menu:

```c
game->encounters_enabled = !game->encounters_enabled;
game->steps_since_encounter = 0;
```

Resetting the counter on toggle means switching encounters back on doesn't
immediately trigger a fight from steps banked while they were off.

### Battle mode in the dispatch table

Adding a fourth mode is, once again, an enum value and a table row:

```c
static const ModeHandler mode_table[MODE_COUNT] = {
    [MODE_EXPLORE] = { "explore", explore_draw, explore_handle },
    [MODE_DIALOG]  = { "dialog",  dialog_draw,  dialog_handle  },
    [MODE_MENU]    = { "menu",    menu_draw,    menu_handle    },
    [MODE_BATTLE]  = { "battle",  battle_draw,  battle_handle  }
};
```

`battle_handle` routes input to the battle and checks the outcome once:

```c
static void battle_handle(Game *game, InputEvent event)
{
    Battle *battle = &game->battle;

    switch (event) {
        case INPUT_UP:      battle_cycle_target(battle, -1); return;
        case INPUT_DOWN:    battle_cycle_target(battle, 1);  return;
        case INPUT_CONFIRM: battle_round(battle, &game->player, &game->rng, 0); break;
        case INPUT_FLEE:    battle_round(battle, &game->player, &game->rng, 1); break;
        case INPUT_QUIT:    game->running = 0; return;
        default:            return;
    }

    if (battle->outcome != BATTLE_ONGOING) {
        battle_finish(game);
    }
}
```

Targeting returns early (no round passes for moving a cursor); attacking
and fleeing advance a round and then fall through to the single outcome
check. `battle_finish` awards XP and gold, reports the result through the
same dialog box NPCs use, and returns to explore mode.

Add `INPUT_FLEE` (`f`) to `input.h` and `input.c`. When you rebuild, the
compiler will remind you about it — see "Common errors" below.

## Compile and run

```bash
make clean
make test
```

```
level table
xp and levelling
battle setup
group size scales with level
battles always end
fleeing always resolves

92398 checks, 0 failed
```

Then play:

```bash
make
./game
```

Walk around until something finds you:

```
-- BATTLE --

> Disgruntled Slime     14



You  HP 20/20  Lv 1

A Disgruntled Slime blocks the way.

Enter attack, w/s target, f flee
```

Enter attacks, `w`/`s` change target when there's more than one, `f` tries
to run. Win and you're paid:

```
Grubbin Vale
 Victory. 3 XP and
 5 gold.
```

And `m` → Encounters toggles them off entirely:

```
Grubbin Vale
   Status
   Commission
 > Encounters: on
   Close menu
```

Verified: with encounters off, 200 steps produced zero fights.

Sanitizer run as always:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

## What just happened — the game was too hard, and measuring proved it

The first time this chapter's code ran, the hero met a Committee of Bats
and died. That could be bad luck. So rather than guess, here is a
simulation harness — a few dozen lines linking the same `battle.c` the game
uses, with no ncurses in sight, running thousands of fights:

```c
for (int t = 0; t < trials; t++) {
    Player p;
    p.level = level;
    party_apply_level(&p);
    p.hp = p.max_hp;

    Battle b;
    battle_begin(&b, &rng, 0, level);

    int rounds = 0;
    while (b.outcome == BATTLE_ONGOING && rounds < 500) {
        battle_round(&b, &p, &rng, 0);
        rounds++;
    }
    if (b.outcome == BATTLE_WON) won++;
}
```

The result was not bad luck:

```
Grubbin Vale encounters, full-HP hero, fight to the death:
  Lv  1 (HP  20 ATK  5 DEF  3 AGI  5): win  17.0%  lose  83.0%
  Lv  2 (HP  26 ATK  6 DEF  4 AGI  6): win  37.8%  lose  62.2%
  Lv  3 (HP  32 ATK  8 DEF  5 AGI  6): win  51.3%  lose  48.7%
```

**A level 1 hero loses 83% of their first fights.** That's not a difficulty
choice, it's a broken opening. So the next question is *why*, and the
harness can answer that too by holding group size fixed:

```
  Lv 1 vs 1 enemies: win  51.6%
  Lv 1 vs 2 enemies: win   0.6%
  Lv 1 vs 3 enemies: win   0.0%

  Lv 3 vs 1 enemies: win  98.8%
  Lv 3 vs 2 enemies: win  53.7%
  Lv 3 vs 3 enemies: win   5.2%
```

There it is. Group size, not enemy stats, is what breaks the early game —
and the reason is structural rather than numerical: **a solo hero attacks
once per round while every enemy attacks.** Two enemies aren't twice as
hard, they're closer to ten times as hard, because they halve your damage
output relative to theirs *and* double their own. Checkpoint B's choice of
a solo hero made this inevitable; the enemy stats were never the problem.

The fix is to cap group size by hero level:

```c
int battle_max_group_for_level(int hero_level)
{
    if (hero_level < 3) {
        return 1;
    }
    if (hero_level < 6) {
        return 2;
    }
    return 3;
}
```

Re-measured:

```
  Lv  1: win  50.4%
  Lv  2: win  91.8%
  Lv  3: win  75.3%     <-- groups of 2 unlock here
  Lv  4: win  96.8%
  Lv  5: win  99.9%
```

A 50/50 first fight, comfortable by level 2, then a deliberate spike at
level 3 when pairs appear, and safety by level 5. That reads like a
difficulty curve instead of a wall. The level 3 dip is a real feature of
the design — the game gets harder exactly when it introduces something
new — and it's the sort of thing you can only see by measuring.

Three lessons worth carrying:

1. **A test suite proves correctness, not fun.** Every one of those 92,398
   assertions passed while the game was unplayable. Balance is a separate
   question requiring separate tools.
2. **Pure, dependency-light modules are measurable.** The harness could
   exist only because `battle.c` doesn't know about ncurses, the world, or
   the game loop — the same property that made Chapter 15's tests possible.
3. **Fifty lines of simulation beat any amount of intuition.** The instinct
   would have been to nerf enemy attack values, which the data shows would
   barely have helped.

Is 50% at level 1 still too harsh? Arguably. It's survivable because
defeat currently just wakes you at 1 HP, and Checkpoint C made difficulty
configurable, so it becomes a knob rather than a verdict — Chapter 17
builds the config file that turns it.

## Common errors

**Adding an enum value and forgetting a switch:**

```
game.c:122:5: warning: enumeration value ‘INPUT_FLEE’ not handled in switch [-Wswitch]
game.c:205:5: warning: enumeration value ‘INPUT_FLEE’ not handled in switch [-Wswitch]
```

This happened while writing this chapter, exactly as Chapter 6 predicted:
adding `INPUT_FLEE` broke two `switch` statements that had no `default:`,
and `-Wswitch` named both. This is the safety net that dispatch tables
*don't* have (Chapter 14) — a good argument for keeping `switch` where the
set of values genuinely is fixed.

**Deleting a dead enemy from the array:**

```c
/* Tempting, and wrong: */
for (int i = index; i < battle->enemy_count - 1; i++) {
    battle->enemies[i] = battle->enemies[i + 1];
}
battle->enemy_count--;
```

Nothing crashes. But `battle->target` was an index into the old
arrangement, so after a kill the player's cursor silently jumps to a
different enemy — and if the last enemy died, `target` now points one past
the end, and `battle->enemies[battle->target]` reads memory that isn't a
live enemy. The `alive` flag avoids the entire class of problem.

**A battle that can't end:**

If `combat_base_damage` could return `0`, `battle_round` would run forever
and the game would hang with no error at all. The damage floor from Chapter
15 is what prevents it, and this is what `test_battle_terminates` exists
to catch:

```c
int rounds = 0;
while (b.outcome == BATTLE_ONGOING && rounds < 500) {
    battle_round(&b, &p, &rng, 0);
    rounds++;
}

CHECK(b.outcome != BATTLE_ONGOING);
CHECK(rounds < 500);
```

The `rounds < 500` bound means a hang becomes a *test failure* rather than
a hung test run — always bound loops in tests that could in principle not
terminate.

## Exercises

> **Practice drills:** `code/ch16/practice/` (`01_defend_flag`–
> `04_win_rate_toy`) before exercise 2.

1. *Practice.* Complete the practice folder — especially defend lifetime
   and reward accumulation on flee.
2. *Durable — defend.* Add a battle `defend` action (`d`) that halves
   incoming damage for one round. State on `Battle`; clear at end of
   round. Keep it.
3. *Durable — flee XP.* Grant accumulated `xp_reward` / `gold_reward` on
   flee (already tallied per kill). Tiny change; keep it.
4. *Measure, don't guess.* Run the chapter simulation harness; optionally
   tweak `battle_max_group_for_level` in a **branch or throwaway build**,
   re-measure, restore the lasting curve you want. Practice `04_win_rate_toy`
   shows why fixed seeds matter for comparison.
5. *Open-ended:* Gate over-level areas — data you have vs something new.

<details>
<summary>Solutions</summary>

1. See `code/ch16/practice/solutions/`.
2. `int defending` on `Battle`; set on action; `damage /= 2` in enemy
   attacks; clear at end of `battle_round`.
3. In flee finish path, grant `battle->xp_reward` (and gold) — already
   accumulated.
4. Decide from the whole win-rate curve, not one level.
5. e.g. warp level gate vs new "danger" flag on areas.

</details>

## Next up

You can fight, win, and level up — but everything you earn is a number
with nowhere to go. Chapter 17 introduces dynamic arrays and `realloc` so
the hero can carry things: potions to survive that 50/50 opening fight,
and eventually the key items that gate the rest of the world. It also
builds the config file Checkpoint C asked for, which is what finally turns
difficulty from a verdict into a setting.
