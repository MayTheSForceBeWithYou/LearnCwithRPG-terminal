# Chapter 17: Menus and Inventory

## Where we are

You can fight, win, level up, and — half the time at level 1 — lose. Every
array in this project so far has had its size decided in advance: four
NPCs per area, three enemies per battle, four menu items. An inventory
can't work that way. You don't know how many different things the hero
will end up carrying, and picking a maximum means either wasting memory or
telling the player "your bag is full" for a reason that's really about
your data structure.

This chapter builds a **dynamic array** that grows as needed, and — since
Chapter 16 proved the game needs tuning — the config file you asked for at
Checkpoint C.

## The problem

`realloc` is the function that resizes a heap allocation, and it is the
single easiest way in C to leak everything you own or lose a pointer to
memory you're still using. It's worth slowing down for.

## C concept spotlight

### The dynamic array

Three fields, always together:

```c
typedef struct {
    ItemStack *stacks;
    int count;      /* stacks in use */
    int capacity;   /* stacks allocated */
} Inventory;
```

`count` and `capacity` are *different numbers* and conflating them is the
classic bug. `capacity` is how many slots exist; `count` is how many hold
real data. Loop to `count`. Grow when `count == capacity`.

```c
void inventory_init(Inventory *inv)
{
    /* No allocation until the first item arrives. A hero who never picks
       anything up never costs a byte of heap. */
    inv->stacks = NULL;
    inv->count = 0;
    inv->capacity = 0;
}
```

Starting at `NULL`/`0`/`0` is deliberate and safe: `realloc(NULL, n)`
behaves exactly like `malloc(n)`, so the growth code needs no special case
for the first allocation.

### `realloc`, and the mistake everyone makes once

```c
static int inventory_grow(Inventory *inv)
{
    int new_capacity = (inv->capacity == 0)
                     ? INVENTORY_INITIAL_CAPACITY
                     : inv->capacity * 2;

    /* Assign to a temporary first. If realloc fails it returns NULL and
       leaves the original block untouched -- writing straight into
       inv->stacks would lose the only pointer to it, leaking everything
       the hero was carrying. */
    ItemStack *grown = realloc(inv->stacks,
                               (size_t)new_capacity * sizeof *grown);
    if (grown == NULL) {
        fprintf(stderr, "inventory_grow: out of memory\n");
        return 0;
    }

    inv->stacks = grown;
    inv->capacity = new_capacity;
    return 1;
}
```

The temporary is the whole lesson. This is the version to never write:

```c
inv->stacks = realloc(inv->stacks, new_size);   /* WRONG */
if (inv->stacks == NULL) { /* too late */ }
```

On failure, `realloc` returns `NULL` **and leaves your original block
allocated and valid**. Assigning the result directly overwrites your only
pointer to that block with `NULL` — the memory is still allocated, you can
no longer reach it, and you can no longer free it. That's a leak of
everything the inventory held, caused by the line that was supposed to
handle the error.

What `realloc` actually does, when it succeeds:

- If it can extend the existing block in place, it does, and returns the
  same address.
- Otherwise it allocates a new block, **copies your data across**, frees
  the old one, and returns the new address.

Which means **any pointer into the old block is dangling after a
successful `realloc`.** Saving `&inv->stacks[0]` across an
`inventory_add` is a use-after-free waiting for the array to grow — the
Chapter 10 bug, arriving through a mechanism that looks nothing like a
`free` call.

### Growth by doubling

Why double, rather than adding one slot at a time?

Adding one slot per insertion means every insertion may copy the whole
array: inserting *n* items costs on the order of *n²* copies. Doubling
means the array only grows at capacities 4, 8, 16, 32 — so *n* insertions
cost on the order of *n* copies total, and each insertion is **amortized
constant time**: individual ones are occasionally expensive, but the
average over many is cheap.

The cost is up to 50% wasted space right after a growth. For an inventory
of a few dozen items that's a handful of bytes. Doubling is the standard
answer; growth factors of 1.5 exist for large buffers where memory reuse
matters more.

### Removal without shuffling

```c
if (stack->quantity == 0) {
    /* Close the gap by moving the last stack into this slot. Order
       is not meaningful here, so this beats shuffling everything. */
    int index = (int)(stack - inv->stacks);
    inv->stacks[index] = inv->stacks[inv->count - 1];
    inv->count--;
}
```

This is **swap-with-last**, and it works only because inventory order
doesn't matter. Shifting every later element down would preserve order at
the cost of copying; swapping with the last is a single assignment. If the
player expected a stable list, you'd need the shift — the point is that
this is a decision, not a default.

`stack - inv->stacks` is **pointer arithmetic**: subtracting two pointers
into the same array yields the number of *elements* between them, not
bytes. The compiler divides by `sizeof(ItemStack)` for you. This only has
meaning for pointers into the same array — anything else is undefined
behaviour.

Note also that this shrinks `count` but never `capacity`. The array never
gives memory back until `inventory_free`. That's a deliberate, common
choice: a hero who once held ten item types will likely do so again, and
shrinking would just mean re-growing later.


### Order-preserving remove vs swap-with-last

Swap-with-last is O(1) and fine for bag sims you never stare at. In a
visible menu, using the selected potion can yank an unrelated stack into
the cursor slot — players notice. Practice both (`02_remove_shift`,
`03_remove_swap`), then pick the lasting policy on purpose.


## Apply it

### The inventory

Items are a lookup table, exactly like enemies and areas:

```c
static const ItemType item_types[ITEM_TYPE_COUNT] = {
    [ITEM_BREAD_RATION]      = { "Bread Ration",       ITEM_EFFECT_HEAL_HP,  25, 0 },
    [ITEM_HERB_POULTICE]     = { "Herb Poultice",      ITEM_EFFECT_HEAL_HP,  60, 0 },
    [ITEM_BOTTLED_VIM]       = { "Bottled Vim",        ITEM_EFFECT_HEAL_MP,  20, 0 },
    [ITEM_BELL_OF_MILD_ALARM]= { "Bell of Mild Alarm", ITEM_EFFECT_FLEE,      0, 0 },
    [ITEM_GUILD_COMMISSION]  = { "Guild Commission",   ITEM_EFFECT_NONE,      0, 1 },
    [ITEM_BRASS_KEY]         = { "Brass Key",          ITEM_EFFECT_NONE,      0, 1 }
};
```

The final field marks key items, which can't be used or dropped — the
Guild Commission and Brass Key exist to gate progress, and letting the
player consume one would strand them.

Items **stack**, so the array holds `{id, quantity}` pairs rather than one
entry per physical item:

```c
int inventory_add(Inventory *inv, ItemId id, int quantity)
{
    if (quantity <= 0 || id < 0 || id >= ITEM_TYPE_COUNT) {
        return 0;
    }

    ItemStack *existing = find_stack(inv, id);
    if (existing != NULL) {
        existing->quantity += quantity;
        return 1;
    }

    if (inv->count == inv->capacity && !inventory_grow(inv)) {
        return 0;
    }

    inv->stacks[inv->count].id = id;
    inv->stacks[inv->count].quantity = quantity;
    inv->count++;

    return 1;
}
```

Returning `int` for success means a caller can react to a failed
allocation rather than assuming the item arrived. Validating the `id`
against the enum's range matters because `ItemId` is just an `int` —
nothing stops `(ItemId)999` reaching this function, and it would index
straight out of the table.

### Items mode

Adding a fifth mode is, once more, an enum value and a table row:

```c
[MODE_ITEMS] = { "items", items_draw, items_handle }
```

Using an item refuses rather than wastes:

```c
case ITEM_EFFECT_HEAL_HP: {
    /* Refuse rather than waste the item -- healing at full HP
       would consume it for nothing, which players rightly read
       as the game stealing from them. */
    if (game->player.hp >= game->player.max_hp) {
        snprintf(line, sizeof line, "You are not hurt.");
        break;
    }

    int before = game->player.hp;

    game->player.hp += type->magnitude;
    if (game->player.hp > game->player.max_hp) {
        game->player.hp = game->player.max_hp;
    }

    int healed = game->player.hp - before;
    inventory_remove_one(inv, id);

    snprintf(line, sizeof line, "%s restores %d HP.", type->name, healed);
    break;
}
```

Reporting `healed` (the actual difference) rather than `type->magnitude`
means a 25 HP ration used at 5 HP below full honestly says "restores 5 HP."

And one line that's easy to forget:

```c
/* Removing an item can shrink the list under the cursor. */
if (game->item_index >= inv->count) {
    game->item_index = inv->count > 0 ? inv->count - 1 : 0;
}
```

Use the last item in your bag and `count` drops; without this, the cursor
points past the end of the array. Any UI that indexes into a collection
that can shrink needs this check.

### The config file (Checkpoint C)

At Checkpoint C you chose difficulty tunable from a file. `assets/config.txt`:

```
# Some Assembly Required -- settings
#
# Every setting below is optional. Delete a line and the game uses its
# default; the whole file can be missing and the game still runs.

# Random encounters: on or off. You can also toggle this in the menu.
encounters = on

# Percent multiplier on damage enemies deal to you. 100 is normal,
# 50 is half, 200 is double. Range 1-1000.
enemy_damage = 100

# Percent multiplier on experience awarded. Range 1-1000.
xp_rate = 100

# Percent of your gold lost when defeated. 0 is forgiving,
# 50 is Dragon Warrior. Range 0-100.
gold_loss_on_death = 0
```

The parser is Chapter 11's file I/O with Chapter 12's string handling, and
its governing rule is that **a bad config must never stop the game**:

```c
int config_load(Config *config, const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        /* Not an error: playing without a config file is normal. */
        return 0;
    }

    char line[CONFIG_MAX_LINE];
    int line_number = 0;

    while (fgets(line, sizeof line, f) != NULL) {
        line_number++;

        char *text = trim(line);

        if (text[0] == '\0' || text[0] == '#') {
            continue;   /* blank line or comment */
        }

        char *equals = strchr(text, '=');
        if (equals == NULL) {
            fprintf(stderr, "%s:%d: expected key = value\n", path, line_number);
            continue;
        }

        *equals = '\0';
        char *key = trim(text);
        char *value = trim(equals + 1);

        /* ... dispatch on key, validate the value, warn and skip if bad ... */
    }

    fclose(f);
    return 1;
}
```

Two techniques worth naming. `strchr` finds the first occurrence of a
character and returns a pointer to it (or `NULL`). Writing `*equals = '\0'`
**splits the line in place** — the buffer now contains two NUL-terminated
strings back to back, `key` and `value`, with no copying and no
allocation. That's a very C way to parse, and it works only because you
own the buffer and don't need the original line afterward.

For numbers, `strtol` beats `atoi` because it reports failure:

```c
static int parse_percent(const char *value, int low, int high, int *out)
{
    char *end = NULL;
    long parsed = strtol(value, &end, 10);

    if (end == value || *end != '\0') {
        return 0;
    }
    if (parsed < low || parsed > high) {
        return 0;
    }

    *out = (int)parsed;
    return 1;
}
```

`strtol` sets `end` to the first character it *couldn't* convert. Two
checks make it strict: `end == value` means nothing was converted at all
(`"banana"`), and `*end != '\0'` means there was trailing garbage
(`"50kg"`). `atoi` silently returns `0` for both, which is
indistinguishable from a legitimate `0`.

Finally the config has to actually *do* something — a setting that's read
and ignored is worse than no setting:

```c
damage = (damage * enemy_damage_pct) / 100;
if (damage < 1) {
    damage = 1;
}
```

Multiply before divide (Chapter 15), and keep the floor so
`enemy_damage = 1` doesn't make the game unloseable.

## Compile and run

```bash
make clean
make test
```

```
inventory growth
inventory removal
inventory stress
config parsing

93460 checks, 0 failed
```

The config tests deliberately feed the parser garbage, so you'll see its
complaints in the output — that's the test working:

```
/tmp/cfg_bad.txt:3: xp_rate must be 1-1000
/tmp/cfg_bad.txt:4: gold_loss_on_death must be 0-100
/tmp/cfg_bad.txt:5: expected key = value
/tmp/cfg_bad.txt:6: unknown setting 'unknown_key'
```

Then play. `m` → Items:

```
Grubbin Vale
 > Guild Commission   x1 (key)
   Bread Ration       x3
```

At full health, the game refuses rather than wasting the item:

```
Grubbin Vale
 You are not hurt.
```

After a fight, it heals honestly and the stack shrinks:

```
Grubbin Vale
 Bread Ration
 restores 5 HP.
```

```
Grubbin Vale
 > Guild Commission   x1 (key)
   Bread Ration       x2
```

Sanitizers matter more than usual this chapter, since `realloc` is
involved — run **both** the game and the tests:

```bash
make clean
make run_tests CFLAGS="-std=c17 -Wall -Wextra -g -fsanitize=address,undefined"
./run_tests
```

```
93460 checks, 0 failed
```

## What just happened

```
  inventory_init:        stacks = NULL   count 0   capacity 0
                         (no heap at all)

  add Guild Commission:  realloc(NULL, 4 * 8) -> malloc
                         stacks -> [Commission x1][ ? ][ ? ][ ? ]
                         count 1   capacity 4

  add Bread Ration x3:   room already
                         stacks -> [Commission x1][Ration x3][ ? ][ ? ]
                         count 2   capacity 4

  add 3 more kinds:      count would hit 5 > capacity 4
                         realloc(stacks, 8 * 8) -- may MOVE the block
                         count 5   capacity 8

  use last Ration:       quantity hits 0, swap-with-last, count 4
                         capacity stays 8
```

The one thing to carry from this diagram: the arrow labelled "may MOVE the
block". Everything else is bookkeeping; that's the part that turns a saved
pointer into a dangling one.

## Common errors

**Assigning `realloc`'s result directly to the pointer you passed in:**

```c
inv->stacks = realloc(inv->stacks, new_size);
```

On failure, the old block is still allocated and you've just overwritten
the only pointer to it. ASan reports it at exit:

```
==65210==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 32 byte(s) in 1 object(s) allocated from:
    #1 ... in inventory_grow inventory.c:52
```

The temporary-then-assign pattern costs one line and removes the entire
failure mode.

**Holding a pointer across a growth:**

```c
ItemStack *first = &inv->stacks[0];
inventory_add(inv, ITEM_HERB_POULTICE, 1);   /* may realloc */
first->quantity++;                            /* use-after-free */
```

```
==65233==ERROR: AddressSanitizer: heap-use-after-free
```

No warning at compile time. The rule: **treat any pointer into a dynamic
array as invalid after any call that can add to it.** Store indices, not
pointers, when the array can grow.

**Looping to `capacity` instead of `count`:**

```c
for (int i = 0; i < inv->capacity; i++) {    /* should be count */
    render_draw_text(1, y + i, item_type(inv->stacks[i].id)->name);
}
```

The slots between `count` and `capacity` were never initialized — `realloc`
doesn't zero new memory any more than `malloc` does — so this reads
uninitialized data and prints whatever it finds:

```
count=1 capacity=4
  slot 0: id=0 qty=1
  slot 1: id=-1094795586 qty=-1094795586
  slot 2: id=-1094795586 qty=-1094795586
  slot 3: id=-1094795586 qty=-1094795586
```

**And here is the important part: AddressSanitizer does not report this
at all.** The run above was built with `-fsanitize=address,undefined` and
produced no diagnostic — just the garbage. ASan tracks memory that is
*out of bounds* or *freed*; these slots are neither. They're memory you
legitimately own and simply never wrote to.

That `-1094795586` is `0xBEBEBEBE` read as a signed int — the same filler
byte you met in Chapter 13. Seeing it is your only clue, and only because
ASan chose a deliberately absurd pattern. Uninitialized-memory reads need
a different tool: Valgrind's memcheck, or Clang's MemorySanitizer.
Chapter 22 introduces Valgrind and hunts exactly this class of bug.

The lesson to carry: **"clean under ASan" does not mean "no memory bugs."**
Each sanitizer has a scope, and this one is outside it.

**`atoi` swallowing garbage:**

```c
config->xp_rate_pct = atoi(value);   /* "banana" -> 0 */
```

An `xp_rate` of `0` from a typo means no XP is ever awarded, and the
player never levels — a bug that looks like a broken progression system
rather than a bad config line. `strtol` with the `end` checks makes it an
error message instead.

## Exercises

> **Practice drills:** `code/ch17/practice/` before exercise 2.

1. *Practice.* Finish add/stack, shift-remove, swap-remove, and grow.
2. *Durable policy.* Choose shift or swap for live `inventory_remove_one`
   and implement it — document the choice in a one-line comment.
3. *Durable — flee item.* `ITEM_EFFECT_FLEE` ends an active battle; outside
   battle, refuse with a clear message. Mode check at the **use site**, not
   in the item table row (table stays data).
4. *Open-ended:* Hot-reload config mid-run — what must wait until battle
   ends?

<details>
<summary>Solutions</summary>

1. Practice solutions under `solutions/`.
2. Shift is usually better UX at inventory sizes; swap is shorter.
3. In `items_use_selected` (or battle use path): if not in battle, message;
   else set flee/outcome. Table only names the effect enum.
4. Refuse combat-affecting knobs until `MODE_WALK` / out of battle.

</details>

## Next up

Your hero has stats, levels, gold, and a bag — and loses all of it the
moment you close the terminal. **Checkpoint E comes next**: before
Chapter 20 you'll choose how the game writes itself to disk. But first,
Chapter 18 and **Checkpoint D**, which decides how magic works — an MP
pool, FF1-style spell charges, or cooldowns — and therefore what the
`Bottled Vim` in your bag is actually for.
