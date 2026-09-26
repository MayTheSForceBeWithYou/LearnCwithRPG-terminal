# Chapter 18: Magic and Equipment

## Where we are

Your hero fights with a fixed attack stat and no options beyond "swing".
At Checkpoint D you chose a single MP pool, a hero who learns every spell
on schedule, and tables that will eventually live in data files. This
chapter spends all three: thirteen spells driven by a table of function
pointers, equipment in three slots that actually changes your stats, and
status effects packed into the bits of a single integer.

`CONTENT.md` §12 asks for one thing specifically — *"implement spell
effects as a data table of function pointers, not a `switch`. This is the
payoff for Chapter 14's dispatch-table lesson — call it back explicitly."*
So: this is Chapter 14's lesson, arriving where it pays.

## The problem

Thirteen spells that heal, burn, freeze, sleep, weaken, harden, hasten,
shield and cure. The obvious implementation is a `switch` with thirteen
cases, and it would work — right up until you want a fourteenth spell, at
which point you're editing a function that's already 200 lines long and
touching code for twelve spells you didn't mean to change.

Equipment has a quieter version of the same problem: if gear bonuses are
added at every place a stat is read, then the one place you forget is a
bug nobody notices until a player asks why their new sword did nothing.

## C concept spotlight

### Bit flags: many booleans in one integer

A combatant can be asleep *and* weakened, or hasted *and* shielded. You
could give `Player` five `int` fields, but there's a tighter way:

```c
#define STATUS_NONE       0u
#define STATUS_STONESKIN  (1u << 0)   /* self: DEF +40%      */
#define STATUS_HASTENED   (1u << 1)   /* self: AGI +50%      */
#define STATUS_WARDED     (1u << 2)   /* self: absorb damage */
#define STATUS_ASLEEP     (1u << 3)   /* enemy: skips turns  */
#define STATUS_DULLED     (1u << 4)   /* enemy: ATK -25%     */
```

`1u << 0` is the number `1` shifted left zero places: binary `00001`.
`1u << 3` is `1` shifted left three places: `01000`. Each constant
occupies **exactly one bit**, and no two overlap:

```
  STATUS_STONESKIN  1u << 0   00001
  STATUS_HASTENED   1u << 1   00010
  STATUS_WARDED     1u << 2   00100
  STATUS_ASLEEP     1u << 3   01000
  STATUS_DULLED     1u << 4   10000

  stoneskin + hastened =      00011   (one unsigned holds both)
```

Three operations do everything:

```c
status |=  STATUS_STONESKIN;   /* set:   turn this bit on            */
status &= ~STATUS_STONESKIN;   /* clear: turn this bit off           */
if (status & STATUS_STONESKIN) /* test:  is this bit on?             */
```

`|` is bitwise OR — it turns a bit on without disturbing the others.
`&` is bitwise AND, used with a single flag to isolate one bit. `~` is
bitwise NOT, which flips every bit, so `~STATUS_STONESKIN` is "all bits
except that one" — ANDing with it clears exactly that flag and preserves
the rest.

Note `unsigned` rather than `int`. Shifting into the sign bit of a signed
integer is undefined behaviour (Chapter 15's territory), and these are
bit patterns, not quantities — using an unsigned type says so.

**The rule that matters:** every flag must be a distinct power of two. Two
flags sharing a bit makes two different statuses indistinguishable, and
nothing warns you. That's worth a test, and there is one below.

### A table of function pointers, again

Chapter 14 replaced a `switch` over game modes with a table of handlers.
This is the same technique aimed at spells:

```c
struct Spell;   /* declared before the type that mentions it */

typedef void (*SpellEffect)(const struct Spell *spell, SpellContext *ctx);

typedef struct Spell {
    const char *name;
    int level;               /* level at which it is learned */
    int mp_cost;
    int magnitude;           /* HP restored, damage dealt, or turns */
    unsigned status_flag;    /* which status it applies, or STATUS_NONE */
    SpellEffect effect;
} Spell;
```

The `struct Spell;` line is a **forward declaration**, and it's needed
because `SpellEffect` mentions `struct Spell` before the struct is
defined. Without it the compiler doesn't know that name yet. (This also
forces the tag `struct Spell` alongside the `typedef` name `Spell` —
one of the few places this course needs both.)

Every effect takes the same two arguments, which is what lets them share
a table. That's why the context is a struct rather than a parameter list:

```c
typedef struct {
    Player *caster;
    int *target_hp;
    unsigned *target_status;
    int *target_turns;
    Rng *rng;
    char message[64];
} SpellContext;
```

A healing spell uses `caster` and ignores the target; a fire spell does
the reverse. Bundling them means adding a *new kind* of spell that needs
something new is one field on the context, not a change to thirteen
signatures. The `message` buffer is how an effect reports what happened
without knowing anything about text boxes or ncurses — the Chapter 8
boundary, still holding.

### The effects, and the table

Five functions cover thirteen spells:

```c
static void effect_heal(const Spell *spell, SpellContext *ctx)
{
    int before = ctx->caster->hp;

    ctx->caster->hp += spell->magnitude;
    if (ctx->caster->hp > ctx->caster->max_hp) {
        ctx->caster->hp = ctx->caster->max_hp;
    }

    snprintf(ctx->message, sizeof ctx->message, "%s restores %d HP.",
             spell->name, ctx->caster->hp - before);
}

static void effect_damage(const Spell *spell, SpellContext *ctx)
{
    /* Spells ignore physical defence entirely (Checkpoint C) but take
       the same variance band as a weapon swing. */
    int roll = rng_range(ctx->rng, 0, 2 * COMBAT_VARIANCE_PERCENT);
    int damage = combat_apply_variance(spell->magnitude, roll);

    *ctx->target_hp -= damage;

    snprintf(ctx->message, sizeof ctx->message, "%s hits for %d.",
             spell->name, damage);
}

static void effect_self_status(const Spell *spell, SpellContext *ctx)
{
    ctx->caster->status |= spell->status_flag;
    ctx->caster->status_turns = spell->magnitude;

    snprintf(ctx->message, sizeof ctx->message, "%s takes hold.",
             spell->name);
}
```

`effect_damage` reusing `combat_apply_variance` from Chapter 15 is not
incidental — it means spells and swords share one variance rule, tested
once, and a change to the band applies to both.

And the table, which is now most of what "magic" means in this game:

```c
static const Spell spell_table[] = {
    /* name            lv  mp  mag  status flag        effect */
    { "Mend",           2,  3,  35, STATUS_NONE,      effect_heal },
    { "Ember",          3,  4,  25, STATUS_NONE,      effect_damage },
    { "Dull",           5,  4,   4, STATUS_DULLED,    effect_enemy_status },
    { "Frost Nip",      6,  6,  40, STATUS_NONE,      effect_damage },
    { "Adjourn",        7,  7,   3, STATUS_ASLEEP,    effect_enemy_status },
    { "Purge",          8,  5,   0, STATUS_NONE,      effect_cure },
    { "Stoneskin",      9,  8,   5, STATUS_STONESKIN, effect_self_status },
    { "Greater Mend",  10, 10, 110, STATUS_NONE,      effect_heal },
    { "Jolt",          12, 11,  70, STATUS_NONE,      effect_damage },
    { "Hasten",        13, 12,   5, STATUS_HASTENED,  effect_self_status },
    { "Immolate",      15, 18, 130, STATUS_NONE,      effect_damage },
    { "Ward",          16, 14,   5, STATUS_WARDED,    effect_self_status },
    { "Sunder",        18, 30, 240, STATUS_NONE,      effect_damage }
};
```

Read that as a spreadsheet, because that's what it is. **Adding a spell is
adding a row. Adding a *kind* of spell is adding a function.** Ember,
Frost Nip, Jolt, Immolate and Sunder are five different spells with
different names, levels, costs and power — and they are all the same six
lines of code.

Because the table is sorted by level, "which spells do I know" is a
prefix, not a search:

```c
int magic_known_count(int hero_level)
{
    int known = 0;

    for (int i = 0; i < SPELL_COUNT; i++) {
        if (spell_table[i].level <= hero_level) {
            known++;
        }
    }

    return known;
}
```

### Guarding the table's assumptions

Data-driven code trades compiler checks for flexibility, so the checks
have to move into the code:

```c
int magic_cast(const Spell *spell, SpellContext *ctx)
{
    if (spell == NULL || ctx == NULL) {
        return 0;
    }

    if (ctx->caster->mp < spell->mp_cost) {
        snprintf(ctx->message, sizeof ctx->message, "Not enough MP.");
        return 0;
    }

    /* A targeted spell with nothing to aim at must not be cast, or the
       effect would dereference a NULL target. */
    if (magic_needs_target(spell) &&
        (ctx->target_hp == NULL || ctx->target_status == NULL)) {
        snprintf(ctx->message, sizeof ctx->message, "Nothing to aim at.");
        return 0;
    }

    ctx->caster->mp -= spell->mp_cost;
    ctx->message[0] = '\0';

    spell->effect(spell, ctx);

    return 1;
}
```

That middle check is load-bearing. Casting Ember from the menu with no
battle in progress leaves `target_hp` as `NULL`, and `effect_damage` would
dereference it — the Chapter 7 crash, arriving through a table. Returning
`0` with a reason lets the caller show "Nothing to aim at." instead.

MP is deducted **before** the effect runs and only after every check has
passed, so there is no path where a spell fires for free or charges you
for a spell that didn't happen.


### Effect tables and per-fighter resources

Spell *kinds* are function pointers (or a switch) keyed by table rows —
adding a row should not edit the battle loop. Resources that change during
a fight (ward absorption left) live on the fighter, not in `spell_table`
(which is immutable definition data). Practice `03_ward_pool` before you
move ward HP onto `Player`.


## Apply it

### Equipment, and one place to read a stat

```c
typedef struct {
    const char *name;
    int attack;
    int defense;
    int agility;
    int cost;
} Gear;

typedef enum {
    SLOT_WEAPON,
    SLOT_ARMOUR,
    SLOT_ACCESSORY,
    SLOT_COUNT
} GearSlot;
```

Three tables from `CONTENT.md` §10, with index 0 of each being the
starting kit so a fresh hero is never holding `NULL`. `Player` stores an
index per slot rather than a pointer — smaller, and it will serialise
trivially when Chapter 20 saves the game.

The important part is that nothing reads `player->attack` directly any
more:

```c
int entity_attack(const Player *p)
{
    int total = p->attack;

    for (int s = 0; s < SLOT_COUNT; s++) {
        const Gear *g = gear_at((GearSlot)s, p->equipped[s]);
        if (g != NULL) {
            total += g->attack;
        }
    }

    return total;
}

int entity_agility(const Player *p)
{
    int total = p->agility;

    for (int s = 0; s < SLOT_COUNT; s++) {
        const Gear *g = gear_at((GearSlot)s, p->equipped[s]);
        if (g != NULL) {
            total += g->agility;
        }
    }

    if (p->status & STATUS_HASTENED) {
        total += total * 50 / 100;
    }

    if (total < 1) {
        total = 1;
    }

    return total;
}
```

Every slot contributes to every stat, which is what makes the
Ledger-Weight (`+5 DEF, -2 AGI`) work without special-casing accessories.
The floor of 1 on agility matters: agility drives turn order, hit chance
and flee chance, and a zero would make all three behave strangely.

Then `battle.c` changes every stat read:

```c
if (!combat_roll_hit(rng, entity_agility(player), enemy->type->agility)) {
int damage = combat_roll_damage(rng, entity_attack(player), 0,
```

Miss one and that path silently ignores your equipment. Searching for
`player->attack` after this change and finding nothing is the check.

### Statuses that do something

Bit flags are only worth having if something reads them:

```c
if (enemy->status & STATUS_ASLEEP) {
    snprintf(line, sizeof line, "The %s sleeps on.", enemy->type->name);
    log_line(battle, line);
    return;
}

int attack = enemy->type->attack;

/* Dull shaves a quarter off the enemy's attack while it lasts. */
if (enemy->status & STATUS_DULLED) {
    attack -= attack / 4;
}
```

```c
/* Ward soaks the blow entirely, once, then breaks. */
if (player->status & STATUS_WARDED) {
    player->status &= ~STATUS_WARDED;
    player->status_turns = 0;
    /* ... report and return ... */
}
```

Ward is the clearest use of `&= ~`: one flag off, everything else
untouched. And timers tick once per round, not once per action:

```c
void magic_tick_status(unsigned *status, int *turns)
{
    if (*status == STATUS_NONE) {
        return;
    }

    if (*turns > 0) {
        (*turns)--;
    }

    if (*turns <= 0) {
        *status = STATUS_NONE;
        *turns = 0;
    }
}
```

### Two modes, and a bug worth showing

`MODE_SPELLS` and `MODE_GEAR` join the dispatch table — two enum values,
two rows, no existing mode touched:

```c
[MODE_SPELLS]  = { "spells",  spells_draw,  spells_handle  },
[MODE_GEAR]    = { "gear",    gear_draw,    gear_handle    }
```

But wiring casting into battle exposed something the earlier modes never
did. Every mode so far returned to `MODE_EXPLORE` when its text box was
dismissed, because every mode was *entered* from exploring. Casting
breaks that: you open the spell list from inside a fight, and when the
"Ember hits for 27." box closes you must land back **in the battle**.

The first version dumped the player onto the map mid-fight. The fix is to
record where the text box came from:

```c
if (event != INPUT_NONE) {
    if (!dialog_advance(&game->dialog)) {
        game->mode = game->dialog_return;
        game->dialog_return = MODE_EXPLORE;
    }
}
```

There was a second bug in the same feature: casting didn't cost a turn, so
you could heal forever while the enemies stood politely still. Attacking
runs a full round; casting has to run *only the enemies' half*, since the
hero has already acted:

```c
void battle_enemy_turn(Battle *battle, Player *player, Rng *rng,
                       int enemy_damage_pct)
{
    if (battle->outcome != BATTLE_ONGOING) {
        return;
    }
    /* ... every living enemy attacks, then statuses tick ... */
}
```

Both bugs share a shape worth recognising: **a new feature made an old
assumption false.** "Dialogs return to explore" and "the hero's action is
always a weapon swing" were true for four chapters and quietly stopped
being true here. Neither produced a warning.

## Compile and run

```bash
make clean
make test
```

```
spell table
spell casting
status bit flags
equipment
enemy-only turn (used when the hero casts)

95651 checks, 0 failed
```

Then the game. `m` → Gear:

```
Grubbin Vale
 > Weapon  Chair Leg
   Armour  Work Clothes
   Charm   (none)
 ATK 7  DEF 4  AGI 5
```

`a`/`d` cycles what's in the selected slot, and the stat line updates
immediately — base attack 5 plus the Chair Leg's 2:

```
Grubbin Vale
 > Weapon  Bronze Shortsword
   Armour  Work Clothes
   Charm   (none)
 ATK 11  DEF 4  AGI 5
```

`m` → Magic shows nothing at level 1, which is correct — Mend arrives at
level 2. After a few fights:

```
Grubbin Vale
 MP 6/6
 > Mend           3 MP
   Ember          4 MP
```

And in a fight, `m` opens the same list aimed at your current target:

```
-- BATTLE --

> Turnip Blight         18
  Disgruntled Slime     --

You  HP 30/32  Lv 3

The Disgruntled Slime misses you.
You hit the Disgruntled Slime for 10.
The Disgruntled Slime falls.
Enter attack, m magic, w/s target, f flee
```

Sanitizers, as always:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

## What just happened

```
  spell_table[1]  "Ember"
  +----------------------------------+
  | name    = "Ember"                |
  | level   = 3      mp_cost = 4     |
  | magnitude = 25                   |
  | status_flag = STATUS_NONE        |
  | effect  ------> effect_damage()  |
  +----------------------------------+
                        |
     magic_cast() checks MP and target, deducts MP, then calls it
                        |
                        v
     effect_damage(spell, ctx)
        reads spell->magnitude          (data)
        applies combat_apply_variance   (shared with weapons)
        writes *ctx->target_hp          (through the context)
        writes ctx->message             (for whoever wants to show it)
```

The function pointer is the only reason `magic_cast` doesn't need to know
what kind of spell it's casting. It checks the universal rules — do you
have the MP, is there something to aim at — and then hands off to a
function selected by data. That's the same shape as `mode_table` in
Chapter 14, and it will be the same shape again when Chapter 21 moves this
table into a file: the *rows* become data on disk, while the *effects*
stay as code, because behaviour can't be serialised.

## Common errors

**Two flags sharing a bit:**

```c
#define STATUS_ASLEEP  (1u << 3)
#define STATUS_DULLED  (1u << 3)   /* same bit! */
```

No warning, no error. Now `status & STATUS_ASLEEP` is true whenever the
enemy is dulled, and clearing one clears both. This is why the test suite
checks every pair:

```c
for (int i = 0; i < 5; i++) {
    CHECK(all[i] != 0);
    for (int j = i + 1; j < 5; j++) {
        CHECK((all[i] & all[j]) == 0);
    }
}
```

**Using `=` instead of `|=` to set a flag:**

```c
status = STATUS_HASTENED;    /* wipes every other status */
status |= STATUS_HASTENED;   /* adds this one            */
```

Both compile. The first silently cures the poison you were suffering from,
which reads as a game balance mystery rather than a bug.

**Clearing with `&` instead of `&= ~`:**

```c
status &= STATUS_WARDED;     /* keeps ONLY ward, clears everything else */
status &= ~STATUS_WARDED;    /* clears ward, keeps everything else      */
```

The missing `~` inverts the meaning completely. If a status change ever
seems to affect unrelated flags, this is the first line to check.

**Forgetting a stat read when adding equipment:**

```c
int damage = combat_roll_damage(rng, player->attack, 0, ...);
```

Compiles, runs, and equipment does nothing on that one path. There is no
tool that finds this for you — the discipline is to grep for the raw field
name after introducing an accessor and confirm every hit is intentional.

## Exercises

> **Practice drills:** `code/ch18/practice/` before exercise 2.

1. *Practice.* Effect table, drain, ward pool, equip-only-if-owned.
2. *Durable — new spell row.* Add one spell to `spell_table` only. How many
   other files changed? (Aim: zero.)
3. *Durable — drain kind or ward pool.* Implement either `effect_drain`
   **or** pooled ward HP (80) as lasting behaviour — rehearse in practice
   first.
4. *Open-ended:* Equip only owned gear using Chapter 17 inventory.

<details>
<summary>Solutions</summary>

1. See practice `solutions/`.
2. Ideally only `magic.c` (or the data file in later chapters).
3. Drain needs caster + target HP on context; ward_left on the fighter.
4. Inventory already tracks ownership — gear UI filters through it.

</details>

## Next up

You can cast, and you can equip — but the gear screen currently hands you
a Silver Rapier for free, which rather undercuts the Guild's forty-gold
advance. Chapter 19 builds shops and inns: gold that means something,
input validation on a purchase, and a bed that restores the MP you just
spent.
