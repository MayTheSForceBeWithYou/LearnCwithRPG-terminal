# Chapter 21: Data-Driven Content

## Where we are

Chapter 16 measured your game and found the early difficulty curve broken.
You fixed it — by editing `battle.c` and rebuilding. Chapter 20 stores a
hero's level in a save file, but the *meaning* of that level still lives in
a `static const` array compiled into the executable.

At Checkpoint D you chose table-driven progression from data files. This
chapter delivers it: the level table and the spell table move to
`assets/`, and rebalancing the game stops requiring a compiler.

## The problem

Every table you've written since Chapter 13 has been the right shape and
the wrong location. `area_table`, the enemy rosters, `level_table`,
`spell_table`, the shop stock — all data, all compiled in. That was the
correct call at the time: a table in C is type-checked, and you weren't
ready to write parsers.

You are now. And the cost of compiled-in content has become concrete —
Chapter 16's balance work needed a rebuild for every experiment.

## C concept spotlight

### What can move to a file, and what cannot

Here is the boundary that governs this whole chapter:

> **Data can be serialised. Behaviour cannot.**

A level row is six integers. Write them as text, read them back, and you
have the same row. But a spell's *effect* is a function — machine code the
linker placed at an address. There is no text form of `effect_damage`.

So a data file can't introduce new behaviour; it can only **select from
behaviour that already exists**:

```c
/* The only effects that exist. A data file selects from this list by
   name; it cannot introduce a new one, because behaviour is code. */
typedef struct {
    const char *name;
    SpellEffect fn;
} EffectBinding;

static const EffectBinding effect_bindings[] = {
    { "heal",         effect_heal },
    { "damage",       effect_damage },
    { "self_status",  effect_self_status },
    { "enemy_status", effect_enemy_status },
    { "cure",         effect_cure }
};

static SpellEffect lookup_effect(const char *name)
{
    for (size_t i = 0; i < sizeof effect_bindings / sizeof effect_bindings[0];
         i++) {
        if (strcmp(effect_bindings[i].name, name) == 0) {
            return effect_bindings[i].fn;
        }
    }

    return NULL;
}
```

That table is the bridge between the file and the program: a string on one
side, a function pointer on the other. It's the same dispatch idea from
Chapter 14, with the selector arriving from disk instead of from an enum.

Which means the division of labour is now:

- **Adding a spell** — a line in `assets/spells.txt`. No compiler.
- **Adding a *kind* of spell** — a new `effect_*` function *and* a row in
  `effect_bindings`. Compiler required.

Being able to state that boundary precisely is most of what "data-driven"
means in practice.

### Fall back, don't fail

A missing data file should not stop the game. The built-in tables stay in
the binary as a fallback:

```c
static const LevelRow builtin_levels[PARTY_MAX_LEVEL] = { /* ... */ };

/* The table actually in use. Starts as a copy of the built-in one, so
   the game works with no data files at all, and is replaced wholesale by
   a successful load. */
static LevelRow level_table[PARTY_MAX_LEVEL];
static int level_table_ready = 0;

static void ensure_table(void)
{
    if (!level_table_ready) {
        memcpy(level_table, builtin_levels, sizeof level_table);
        level_table_ready = 1;
    }
}
```

Note `builtin_levels` is `const` and `level_table` is not — one is the
reference copy, the other is working storage. `ensure_table` is called
at the top of every public function, so the table is always valid whether
or not anyone loaded anything. That's **lazy initialisation**, and the
`level_table_ready` flag is what stops it re-copying (and discarding a
loaded table) on every call.

Deleting both data files produces a warning and a playable game:

```
levels: no assets/levels.txt, using built-in levels
spells: no assets/spells.txt, using built-in spells
```

### Parsing a whole line: `%n` and the rest of the string

Spell names contain spaces — "Frost Nip", "Greater Mend". `sscanf`'s `%s`
stops at the first space, so the name can't be just another field. The fix
is to parse the fixed fields, ask where parsing stopped, and take
everything after that:

```c
char keyword[16], status_name[16], effect_name[16];
int lv, mp, mag;
int consumed = 0;

if (sscanf(p, "%15s %d %d %d %15s %15s %n",
           keyword, &lv, &mp, &mag, status_name, effect_name,
           &consumed) != 6 || strcmp(keyword, "spell") != 0) {
    /* ... reject ... */
}

/* %n gives where the name begins; the rest of the line is it. */
char *name = p + consumed;
```

`%n` is unusual: it doesn't consume input, it **writes the number of
characters consumed so far** into an `int *`. And because it converts
nothing, it isn't counted in `sscanf`'s return value — which is why the
check is `!= 6` for seven format specifiers. Getting that count wrong is a
classic `%n` bug.

The width limits (`%15s` into a 16-byte buffer) are not optional. A bare
`%s` into a fixed buffer is exactly the unbounded copy Chapter 12 warned
about, and `sscanf` will happily overflow it.

### Validating structure, not just fields

Chapters 11, 17 and 20 validated individual values. Tables need more,
because a table has properties no single line can express.

**Every level must be present:**

```c
for (int i = 0; i < PARTY_MAX_LEVEL; i++) {
    if (!seen[i]) {
        snprintf(reason, reason_size, "%s: level %d is missing",
                 path, i + 1);
        failed = 1;
        break;
    }
}
```

**No level may appear twice**, or the file silently has two answers.

**The cap must terminate:**

```c
/* The cap must terminate progression or party_gain_xp would spin. */
if (!failed && parsed[PARTY_MAX_LEVEL - 1].xp_to_next != 0) {
    snprintf(reason, reason_size, "%s: level %d must have xp_to_next 0",
             path, PARTY_MAX_LEVEL);
    failed = 1;
}
```

That check exists because of a specific hazard: `party_gain_xp` loops
while the hero has enough XP for the next level. A non-zero `xp_to_next`
at level 20 makes that loop's exit condition depend on `p->level <
PARTY_MAX_LEVEL` alone — which does still hold — but the intent is now
ambiguous and one edit away from a hang. A data file can express states
your code was never written to handle; the loader is where you refuse them.

**Spells must stay sorted by level**, because `magic_known_count` counts a
prefix rather than searching:

```c
/* The known-spell count relies on the list being sorted. */
if (count > 0 && lv < parsed[count - 1].level) {
    snprintf(reason, reason_size, "%s:%d: spells must be sorted by level",
             path, line_number);
    failed = 1;
    break;
}
```

This is the important habit: **when you move a table to a file, write down
the invariants the code was silently relying on, and check them.** In C
they were enforced by you being the only author. In a file, anyone is.

### Owning your strings

One change to `Spell` matters more than it looks:

```c
typedef struct Spell {
    /* Owned storage rather than a pointer, so a spell read from a file
       does not depend on a buffer that has since been reused. */
    char name[MAGIC_NAME_LEN];
    ...
```

It was `const char *name`, pointing at a string literal — fine, because
literals live forever. A name parsed from a file lives in `line`, a stack
buffer that `fgets` overwrites on the very next iteration. Storing the
pointer would leave every spell naming whatever the last line happened to
be. Copying costs 24 bytes per spell and removes an entire class of
dangling-pointer bug.

## Apply it

### `assets/levels.txt`

```
# Level table for Some Assembly Required
#
# Format:  level <n> <hp> <mp> <atk> <def> <agi> <xp_to_next>
# xp_to_next of 0 marks the level cap.
#
# These numbers are a starting point and WILL need playtesting.
# Chapter 16 measured the early curve and had to change it.
# Edit freely -- the game re-reads this file every time it starts.

level  1    20    0    5    3    5       8
level  2    26    2    6    4    6      18
level  3    32    4    8    5    6      32
...
level 20   240   85   52   41   27       0
```

### `assets/spells.txt`

```
# Format:  spell <level> <mp> <magnitude> <status> <effect> <name...>
#
# status:  none stoneskin hastened warded asleep dulled
# effect:  heal damage self_status enemy_status cure
#
# The EFFECT names map to functions compiled into the game -- behaviour
# cannot be written in a data file, only selected. Adding a new effect
# still means writing C; adding a new SPELL does not.
#
# Must stay sorted by level: the game treats known spells as a prefix.

spell  2   3   35  none       heal          Mend
spell  3   4   25  none       damage        Ember
spell  5   4    4  dulled     enemy_status  Dull
spell  6   6   40  none       damage        Frost Nip
...
spell 18  30  240  none       damage        Sunder
```

The comments are part of the deliverable. A data file is an interface for
a human, and the format is only obvious to the person who just wrote the
parser.

### Loading them — and one ordering bug worth keeping

```c
config_defaults(&game->config);
config_load(&game->config, "assets/config.txt");

/* Content tables come from disk when they are there, and fall back
   to the built-in copies when they are not. Either way the game
   starts -- a missing data file is a downgrade, not a failure.

   This must happen BEFORE the hero is created, because creating a
   hero reads the level table for their starting stats. Loading it
   afterwards leaves a level 1 hero built from the old numbers. */
{
    char reason[96];

    if (!party_load_levels(PARTY_LEVELS_PATH, reason, sizeof reason)) {
        fprintf(stderr, "levels: %s\n", reason);
    }
    if (!magic_load_spells(MAGIC_SPELLS_PATH, reason, sizeof reason)) {
        fprintf(stderr, "spells: %s\n", reason);
    }
}

game->player = entity_create_player();
```

That comment records a real bug from writing this chapter. The loaders were
originally called *after* `entity_create_player`, which meant everything
looked right — the file loaded, no warnings, tests passed — and edits to
`levels.txt` had no visible effect, because the hero's starting stats had
already been read from the built-in table. Loading data and *using* it are
different moments, and only the second one is observable.

## Compile and run

```bash
make clean
make test
```

```
data-driven tables
data tables reject bad files
retuning by editing a file

101305 checks, 0 failed
```

Now the payoff. Build once and note the time:

```bash
make
./game
```

```
HP:     20/20
ATK:    7
```

Edit `assets/levels.txt` — change level 1 to `75` HP and `12` attack —
and run the **same binary** again, with no rebuild:

```
HP:     75/75
ATK:    14
```

```
binary timestamp: 2026-08-22 17:28:53   (unchanged)
```

That's the whole chapter in two numbers. `ATK 14` is 12 from the edited
table plus 2 from the Chair Leg, so the gear maths still layers on top
correctly.

Deleting the files still works:

```
levels: no assets/levels.txt, using built-in levels
spells: no assets/spells.txt, using built-in spells
```

And a corrupt file is refused with the built-in table left in place:

```
levels: assets/levels.txt:1: nonsensical stats
```

Sanitizers:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

## What just happened

```
  assets/spells.txt                        magic.c
  ------------------                       -------
  spell 3 4 25 none damage Ember
             |    |      |    |
             |    |      |    +-- name: COPIED into the Spell
             |    |      |
             |    |      +------- "damage" ---> lookup_effect()
             |    |                                  |
             |    |                                  v
             |    |                          effect_bindings[]
             |    |                          { "damage", effect_damage }
             |    |                                  |
             |    +---- magnitude, mp -------+       |
             |                               v       v
             +-- level ------------> spell_table[1] = {
                                        name "Ember", level 3,
                                        mp 4, magnitude 25,
                                        effect ---> effect_damage (code)
                                     }

  The file supplies WHICH and HOW MUCH.
  The binary supplies WHAT IT DOES.
```

Everything above the line is editable without a compiler. Everything below
it is C. The `effect_bindings` table is the only place they meet, and it's
deliberately small — five entries — because each one is a promise that a
name in a file will keep working.

## Common errors

**Storing a pointer into the line buffer:**

```c
parsed[count].name = name;   /* name points into `line` */
```

`line` is reused by the next `fgets`, so every spell ends up named after
the last line of the file — or worse, after freed stack memory once the
function returns. Copy the string.

**Miscounting `sscanf`'s return with `%n`:**

```c
if (sscanf(p, "%15s %d %d %d %15s %15s %n", ..., &consumed) != 7) {
```

Seven specifiers, but `%n` converts nothing and isn't counted, so a
perfectly good line fails this check and every spell is rejected. The
correct comparison is `!= 6`.

**Unbounded `%s`:**

```c
char keyword[16];
sscanf(p, "%s ...", keyword);   /* no width limit */
```

A long first word overflows `keyword`. Under ASan:

```
==71204==ERROR: AddressSanitizer: stack-buffer-overflow
```

Always give `%s` a width one smaller than the buffer.

**Loading data after using it:**

The ordering bug above. It produces no error, no warning, and no
diagnostic — the file loads successfully and the effect simply doesn't
appear. If a data change seems to be ignored, check *when* it's read
relative to when it's used, before suspecting the parser.

**Forgetting that a file can express states your code can't handle:**

A level table where level 20 still awards XP, spells out of order, a
duplicated level — none of these are possible when you write the table in
C and read it once. All are possible in a file. The loader is the only
place to catch them.

## Exercises

1. Add a spell to `assets/spells.txt` — a level 4 heal called
   "Second Wind", say. Confirm it appears in the game with no rebuild.
   Now add one with the effect name `teleport`. What happens, and is the
   message enough to fix it?
2. `magic_known_count` relies on the spell list being sorted, and the
   loader enforces it. Remove that check, put a level 18 spell first in
   the file, and describe exactly what the player sees. Why is a loader
   check better than making `magic_known_count` sort or search?
3. Move the enemy rosters from `battle.c` to `assets/enemies.txt`. What
   invariant does that table have that the level table doesn't? (Hint:
   what does `battle_begin` assume about a roster before picking from it?)
4. *Open-ended:* The area table in `world.c` holds NPC dialogue, which is
   the content most likely to need editing. Sketch the file format —
   remembering that speech contains spaces, apostrophes and possibly
   newlines — and say which part would be hardest to parse safely.

<details>
<summary>Solutions</summary>

1. Adding `spell  4   4   45  none  heal  Second Wind` works immediately —
   run the game and a level 4 hero knows it. Using `teleport` fails the
   load and reports:
   ```
   spells: assets/spells.txt:14: unknown effect 'teleport'
   ```
   That names the file, the line, and the offending word, which is enough
   to fix it — and the game still runs on the built-in list. The message
   would be better still if it listed the valid effect names; that's a
   small improvement worth making, since the file's comment block can
   drift out of sync with `effect_bindings`.

2. With an unsorted file, `magic_known_count(2)` counts every spell whose
   level is `<= 2` by scanning the whole list — so it would return 1 for a
   list starting with Sunder followed by Mend, but `magic_spell_at(0)`
   returns *Sunder*. The player's spell menu shows one entry, and it's the
   level 18 nuke, castable at level 2 if they have the MP. A loader check
   is better than sorting because it fixes the problem at the boundary,
   once, at startup — sorting inside `magic_known_count` would run on every
   frame the menu is open, and searching would make the function O(n) for
   no benefit. Validate at the edge; keep the interior simple.

3. Each roster needs `max_group`, and crucially `battle_begin` calls
   `rng_range(rng, 0, roster->count - 1)` to pick an enemy. **A roster
   with zero enemies makes that `rng_range(rng, 0, -1)`** — which
   `rng_range` handles (it returns `low` when `high < low`), but which
   would then index `roster->types[0]` on an empty array. The level table
   has no equivalent hazard because it's fixed-size. So the enemy loader
   must reject a roster that declares itself non-empty but lists nothing,
   and areas with no roster must be expressible deliberately (as
   Castle Hollis already is with `NULL`).

4. No fixed answer. The hard part is speech: it contains spaces (so it must
   be the rest of the line, like spell names), apostrophes (fine in a text
   file, but a reminder not to invent quoting you don't need), and
   ideally line breaks for long dialogue — which a line-based format
   cannot express without either a continuation marker or a
   multi-line block syntax with a terminator. The safest design is the
   dullest: one line per speech, and let `dialog_wrap` (Chapter 12) handle
   presentation, since it already pages arbitrary-length text. Inventing a
   quoting scheme is where hand-rolled parsers usually acquire their
   security bugs.

</details>

## Next up

You have written six parsers, four dispatch tables, three dynamic
allocations and about 3,600 lines of C, and you have been getting away
with things. Chapter 22 finds out which ones: the entire game under
`valgrind` and the sanitizers, with assertions added where assumptions
live, hunting the bugs that twenty-one chapters have quietly accumulated.
