# Chapter 20: Saving the Game

## Where we are

Nineteen chapters of progress — a level, spells, gear, a bag, gold — and
all of it evaporates when you close the terminal. At Checkpoint E you
chose text saves, written at inns, with a corrupt file refused rather than
partially loaded. This chapter builds exactly that.

## The problem

Saving looks like the easiest thing in the game: open a file, write some
numbers, close it. The difficulty is entirely on the **read** side. A save
file is input, and Chapter 11 established what that means — input lies.
Except this time the input is a file *your own program wrote*, which makes
it tempting to trust, and that temptation is the bug.

A save can be truncated by a full disk, edited by a curious player,
written by an older version of your game, or corrupted by something you'll
never diagnose. Every one of those must fail safely.

## C concept spotlight

### Serialisation is a format decision, not an I/O problem

**Serialisation** is turning your in-memory state into a byte sequence you
can store, and back again. The `fopen`/`fgets`/`strtol` mechanics are all
things you already know from Chapters 11 and 17. What's new is deciding
*what shape* those bytes take, and that decision has consequences that
outlive the code.

Checkpoint E chose text:

```
# Some Assembly Required -- save file
# Edit at your own risk. A field out of range is refused.
version = 1
name = Wick
area = 0
x = 8
y = 6
level = 4
xp = 12
gold = 28
hp = 25
mp = 6
weapon = 1
armour = 1
accessory = 0
rng = 424242
item = 0 2
```

You can read that. You can fix a broken one in an editor. You can diff two
saves to see what a fight changed. Those properties are worth a great deal
during development and cost only file size, which is irrelevant at this
scale.

### The road not taken: why `fwrite` of a struct is a trap

`DESIGN.md` offered binary saves partly because they teach **struct
padding** and **endianness**, and Checkpoint E declined them. They're worth
understanding anyway, because the temptation is real — binary saving looks
like one line:

```c
fwrite(&player, sizeof player, 1, f);   /* please do not */
```

Consider what that line actually writes. Take a small struct:

```c
typedef struct {
    char  rank;      /* 1 byte  */
    int   hp;        /* 4 bytes */
    char  initial;   /* 1 byte  */
    int   gold;      /* 4 bytes */
} SaveRecord;
```

Ten bytes of data. Measured:

```
sizeof(char) = 1, sizeof(int) = 4
1 + 4 + 1 + 4 = 10 bytes of actual data
sizeof(SaveRecord) = 16

offsetof rank    = 0
offsetof hp      = 4
offsetof initial = 8
offsetof gold    = 12
```

**Sixteen bytes**, and the offsets show why. `hp` starts at byte 4, not
byte 1 — the compiler inserted three bytes of nothing after `rank` so that
`hp` lands on a 4-byte boundary. This is **struct padding**: most CPUs read
aligned values faster (and some architectures fault on unaligned access),
so compilers silently insert gaps. Dump the raw bytes and the padding is
visible:

```
raw bytes of a record (rank='E', hp=20, initial='W', gold=40):
45 00 00 00 14 00 00 00 57 00 00 00 28 00 00 00
```

`45` is `'E'`, then three padding bytes. `14` is 20. `57` is `'W'`, then
three more.

Three things follow, and each one is a real bug someone has shipped:

- **The padding bytes are uninitialised.** You are writing whatever
  happened to be in that memory into your save file. Harmless for a game;
  a genuine information leak in other software.
- **Reordering the struct changes the file format.** Move `initial` above
  `hp` to save space and every existing save silently misreads.
- **Adding a field changes `sizeof`.** Every old save is now the wrong
  length, and nothing tells you except nonsense values.

Then there's the last line of that measurement:

```
endianness: the int 20 is stored as 14 00 00 00
```

The value 20 is `0x14`, and it's stored **least-significant byte first** —
this machine is **little-endian**. A big-endian machine stores `00 00 00
14`. A binary save written on one and read on the other gives you
335,544,320 gold. That's a bug you cannot reproduce on your own hardware.

None of this makes binary formats wrong — real ones exist, and they solve
these problems with explicit field-by-field writing, fixed-width types, and
a declared byte order. What's wrong is `fwrite(&struct)`, which looks like
it solved the problem and has instead deferred it.

Text has none of these issues. `gold = 28` means the same thing on every
machine, forever.

### Versioning

The first line of the file, and the first thing checked:

```c
#define SAVE_VERSION 1
```

```c
if (tmp.version != SAVE_VERSION) {
    snprintf(reason, reason_size,
             "Save is version %d, this game reads %d.",
             tmp.version, SAVE_VERSION);
    failed = 1;
}
```

A save from a future format is *detected* rather than misread. Without
this, adding a field later means old saves load with garbage in the new
field and the player never learns why their game is strange.

Note what's checked at the *end* as well:

```c
/* Required fields must actually have appeared. A file missing its
   version line is not a version 0 save, it is not a save. */
if (!failed && tmp.version != SAVE_VERSION) {
    snprintf(reason, reason_size, "Save has no version line.");
    failed = 1;
}
```

`tmp.version` was initialised to `-1`, so a file with no version line fails
this check. **Absent is not the same as zero**, and a parser that can't
tell them apart will happily accept a text file that isn't a save at all.

### Validate, then commit — now for a whole file

Chapter 19 established the rule for a purchase. A load is the same rule at
larger scale: read everything into locals, check everything, and only then
touch the caller's data.

```c
SaveData tmp;
memset(&tmp, 0, sizeof tmp);
tmp.version = -1;
tmp.level = -1;

Inventory tmp_inv;
inventory_init(&tmp_inv);

/* ... parse every line into tmp / tmp_inv, setting failed on any problem ... */

if (failed) {
    inventory_free(&tmp_inv);
    return 0;
}

/* Everything checked. Now commit. */
inventory_free(inv);
*inv = tmp_inv;
*save = tmp;
```

That structure is what makes "no partial load" true rather than aspirational.
A file that is valid for fourteen lines and garbage on the fifteenth leaves
the running game *completely untouched* — the player keeps playing the
character they had. Parsing straight into the live `Player` would leave
them with a half-loaded hybrid, which is worse than either outcome.

The `inventory_free(&tmp_inv)` on the failure path matters too: the
temporary inventory may have grown before the bad line was reached, and
abandoning it would leak (Chapter 17).

### Deriving rather than trusting

The save stores `level`, but not `max_hp` or `attack`:

```c
/* Derived stats come from the level table, never from the file, so a
   hand-edited save cannot invent a hero with 9999 attack. */
party_apply_level(&game->player);

game->player.hp = data.hp > game->player.max_hp
                ? game->player.max_hp : data.hp;
```

This is a design decision worth naming. **Store the minimum that cannot be
recomputed; derive everything else.** It makes the file smaller, keeps
saves valid when you rebalance the level table, and closes the door on a
whole class of hand-edited cheating — because `attack` isn't a field
anyone can edit.

The `hp` clamp is the same instinct: `hp` genuinely must be stored (you
can be hurt at any level), but it's clamped against the derived maximum
rather than trusted, so a save claiming 9999 HP at level 4 gets 39.


### Write and read change together; replace atomically

Every new field must land in **both** `save_write` and `save_read`, or the
strict unknown-key policy rejects your own saves. Prove it with a
round-trip test (practice `02`). On disk, write `save.tmp` then `rename`
over the real path so a crash mid-write cannot leave a half file as the
only save (practice `04`).


## Apply it

### `save.h`

```c
#define SAVE_VERSION 1
#define SAVE_PATH "assets/save.txt"

typedef struct {
    int version;
    char name[64];
    int area, x, y;
    int level, xp, gold, hp, mp;
    int equipped[SLOT_COUNT];
    unsigned rng_state;
} SaveData;

int save_write(const char *path, const SaveData *save, const Inventory *inv,
               char *reason, size_t reason_size);

/* Reads a save. On success fills *save and *inv and returns 1. On ANY
   problem -- missing file, wrong version, bad field, unknown key --
   returns 0 with a specific reason, having left *save and *inv
   untouched. There is no partial load. */
int save_read(const char *path, SaveData *save, Inventory *inv,
              char *reason, size_t reason_size);
```

`SaveData` is a plain snapshot rather than pointers into the live game, so
writing a save can't be confused with mutating one. The `reason` buffer
with its size is the Chapter 19 convention, unchanged.

Storing `rng_state` is what `CONTENT.md` §9.4 asked for back in Chapter 15:
the whole generator is one number, so a run can be replayed exactly.

### One easily-missed check when writing

```c
/* fclose can fail -- the bytes may still be in a buffer that never
   reached the disk. Reporting success without checking would mean
   telling the player their game is saved when it is not. */
if (fclose(f) != 0) {
    snprintf(reason, reason_size, "Failed to finish writing %s.", path);
    return 0;
}
```

`fprintf` writes into a buffer; the buffer reaches the disk when it fills
or when the file is closed. A disk that fills up mid-save typically
reports the error at `fclose`, not before. Ignoring `fclose`'s return is
how a program cheerfully announces "Saved." over a truncated file.

### Saving at the inn

Checkpoint E chose inns as the save point, and Chapter 19's exercise 3
already worked out where the call belongs:

```c
} else if (shop_rest(shop, &game->player, reason, sizeof reason)) {
    /* Checkpoint E: inns are the save point. The save call lives
       here, in the caller that holds the whole Game, rather than
       inside shop_rest -- which knows only about a Shop and a
       Player, and should stay that way. */
    char save_reason[48];

    if (game_save(game, save_reason, sizeof save_reason)) {
        snprintf(reason, sizeof reason,
                 "You sleep. HP and MP restored, and recorded.");
    } else {
        snprintf(reason, sizeof reason, "You sleep, but: %s", save_reason);
    }
}
```

Note the failure branch: if the save fails, the player is *told*, and the
rest still happened. Silently failing to save is the one outcome that must
not be possible.

### Loading at startup

```c
if (save_exists(SAVE_PATH)) {
    char reason[96];

    printf("\nA saved game exists. Continue it? (y/n) ");
    /* ... read the answer ... */

    if (answer == 'y' || answer == 'Y') {
        if (game_load(game, reason, sizeof reason)) {
            printf("%s Welcome back.\n", reason);
        } else {
            printf("Could not load: %s\n", reason);
            printf("Starting a new game instead.\n");
        }
    }
}
```

A refused load is not fatal. The reason is printed, a new game begins, and
the bad file is left alone for you to inspect.

## Compile and run

```bash
make clean
make test
```

```
save round trip
save refuses every malformed file
a refused load changes nothing

101224 checks, 0 failed
```

Play, take some damage, then sleep at Bess Harrow's:

```
Grubbin Vale
 You sleep. HP and MP
 restored, and
 recorded.
```

Quit, restart, and the game offers your run back:

```
A saved game exists. Continue it? (y/n) y
Loaded. Welcome back.

Welcome, Wick
----------------------------------------
Level:  4
HP:     25/39
MP:     6/6
Gold:   28
ATK:    15
DEF:    10
----------------------------------------
```

`ATK 15` is base 9 from the level table plus 6 from the Bronze Shortsword —
equipment survived the trip, derived from the stored slot index rather than
stored directly.

Now corrupt it on purpose. Open `assets/save.txt`, change `level = 4` to
`level = 999`, and restart:

```
Could not load: Line 7: bad value for 'level'.
Starting a new game instead.
```

Sanitizers:

```bash
make clean
make CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined" \
     LDLIBS="-lncursesw -fsanitize=address,undefined"
./game
```

## What just happened

Seventeen deliberately broken files were fed to the loader; every one was
refused with a specific reason:

```
  valid (should LOAD)          -> LOADED (Loaded.)
  wrong version                -> refused (Save is version 99, this game reads 1.)
  no version line              -> refused (Save has no version line.)
  no level line                -> refused (Save has no level line.)
  no name line                 -> refused (Save has no name line.)
  level out of range           -> refused (Line 3: bad value for 'level'.)
  level zero                   -> refused (Line 3: bad value for 'level'.)
  negative gold                -> refused (Line 4: bad value for 'gold'.)
  non-numeric gold             -> refused (Line 4: bad value for 'gold'.)
  trailing garbage             -> refused (Line 4: bad value for 'gold'.)
  area out of range            -> refused (Line 4: bad value for 'area'.)
  weapon out of range          -> refused (Line 4: bad value for 'weapon'.)
  unknown key                  -> refused (Line 4: unknown key 'wings'.)
  no equals sign               -> refused (Line 4: expected key = value.)
  bad item id                  -> refused (Line 4: bad item entry.)
  bad item qty                 -> refused (Line 4: bad item entry.)
  item missing qty             -> refused (Line 4: bad item entry.)
  empty file                   -> refused (Save has no version line.)
```

"Trailing garbage" is the one worth dwelling on: `gold = 50kg` is refused
because `strtol` reports where it stopped and the parser insists the whole
value converted. `atoi` would have returned `50` and lost the `kg` — the
same lesson as Chapter 17's config parser, now guarding a file the player
is explicitly invited to edit.

## Common errors

**Trusting a file your own program wrote:**

```c
fscanf(f, "%d", &player->level);   /* straight into the live player */
```

Nothing validates, nothing bounds-checks, and a partial parse leaves the
player half-loaded. That the file *came from* your program is not evidence
about what's in it now.

**Treating a missing field as zero:**

```c
SaveData tmp;
memset(&tmp, 0, sizeof tmp);    /* version is now 0 */
/* ... parse ... */
if (tmp.version != SAVE_VERSION) { /* a file with no version reads as 0 */ }
```

This *appears* to work — a missing version fails the check. But it reports
"version 0" rather than "no version line", which sends you looking for a
version-0 format that never existed. Initialising to a sentinel like `-1`
lets the two cases be distinguished.

**Ignoring `fclose`:**

```c
fprintf(f, "gold = %d\n", save->gold);
fclose(f);                      /* return value discarded */
return 1;                       /* "Saved." -- maybe */
```

No warning. The failure only appears when the disk is full or the
filesystem is remote, which is to say: not on your machine, only on
someone else's.

**Storing derived values:**

```
max_hp = 39
attack = 15
```

Now rebalancing the level table breaks every existing save, and anyone can
edit `attack` to 9999. Store `level`; compute the rest.

## Two bugs this chapter's testing exposed

Loading a save surfaced two defects that had been in the game since
Chapter 18, both invisible until now.

**The character sheet showed base stats, not effective ones.** Chapter 18
introduced `entity_attack`, converted `battle.c` to use it, and was
supposed to convert `entity_print_sheet` too — but that edit silently
didn't apply, and nothing caught it. The sheet said `ATK 9` while the hero
hit for 15. It surfaced here only because loading a save with equipment
made the discrepancy obvious.

**The name printed without a line break.** `entity_create_player` used
`fgets`, which keeps the newline, and `entity_print_sheet` quietly relied
on that newline for its formatting:

```c
printf("Welcome, %s", p->name);   /* the \n came from fgets */
```

A name loaded from a save has no trailing newline, so the layout collapsed
into `Welcome, Wick------------`. The fix is to trim the newline once, at
the point of input, and format explicitly:

```c
/* fgets keeps the newline. Trim it here, once, so every later use of
   the name is a plain string -- printing it, saving it, loading it
   back. Relying on that trailing newline for formatting broke the
   moment a name came from a save file instead of the keyboard. */
size_t len = strlen(p.name);
while (len > 0 && (p.name[len - 1] == '\n' || p.name[len - 1] == '\r')) {
    p.name[len - 1] = '\0';
    len--;
}
```

Both are the same lesson in different clothes: **data acquiring a second
source exposes assumptions the first source was quietly satisfying.** The
newline was never a property of the name; it was a property of `fgets`,
borrowed for formatting until something else supplied the name.

## Exercises

> **Practice drills:** `code/ch20/practice/` before exercise 2.

1. *Practice.* Version header, round-trip blob, unknown-key refuse, atomic
   replace.
2. *Durable — new field.* Add `steps_since_encounter` to **both** write and
   read; validate a sane range; extend the round-trip test.
3. *Durable — atomic save.* Write `save.tmp`, then `rename` onto the real
   save path after a successful close.
4. *Open-ended:* Suspend save (anywhere, delete on load) vs inn-only —
   what in `save.c` is reused unchanged? (Version bump/migration sketch
   optional.)

<details>
<summary>Solutions</summary>

1. Practice solutions.
2. Forget read → unknown key on load. Round-trip test catches it.
3. `fopen` tmp → write → `fclose` → `rename(tmp, final)`.
4. Serialize/parse helpers reused; path and delete-on-load policy differ.

</details>

## Next up

Your level table and spell table are still compiled into the program, so
retuning the balance problem Chapter 16 measured means a rebuild.
Checkpoint D said those tables move to data files — Chapter 21 does it,
using the parsing you've now written three times, and turning the game's
content into something you can edit while it's closed.
