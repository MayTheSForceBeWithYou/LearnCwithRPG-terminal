# Chapter 19: Shops and Economy

## Where we are

Chapter 18 gave your hero thirteen spells and three equipment slots — and
handed out a Silver Rapier for free, which rather undercuts the Guild's
forty-gold advance. Gold has accumulated from every fight since Chapter 16
and bought exactly nothing. This chapter closes that loop: shopkeepers who
sell, an inn that restores the MP you spent casting, and the input
validation that keeps a purchase from going wrong.

## The problem

A transaction is the most failure-prone thing in the game so far, because
it has to be **all-or-nothing**. Take the gold but fail to deliver the
item and you've robbed the player. Deliver the item but fail to take the
gold and the economy is meaningless. Take the gold for something they
already own and they'll notice immediately.

None of these crash. They're all just wrong, quietly, in a way that
erodes trust in the game rather than breaking it.

## C concept spotlight

### Validate, then commit

Every operation in this chapter follows one shape: **check everything
that could fail, then make the change**. Never interleave them.

```c
int shop_buy(const StockEntry *entry, Player *player, Inventory *inv,
             char *reason, size_t reason_size)
{
    if (entry == NULL || player == NULL || inv == NULL) {
        snprintf(reason, reason_size, "Nothing to buy.");
        return 0;
    }

    int price = shop_entry_price(entry);
    const char *name = shop_entry_name(entry);

    /* Check before charging, always. */
    if (player->gold < price) {
        snprintf(reason, reason_size, "%s costs %d. You have %d.",
                 name, price, player->gold);
        return 0;
    }
    ...
```

The gold is not touched until every reason to refuse has been ruled out.
That ordering is the entire technique, and it's why there is no path
through this function that charges for a failed purchase.

The consumable branch shows the subtler version, because storing an item
can *itself* fail:

```c
    /* Consumables can fail to be stored if the inventory cannot grow, so
       take the money only once the item is safely in the bag. */
    if (!inventory_add(inv, (ItemId)entry->index, 1)) {
        snprintf(reason, reason_size, "You cannot carry any more.");
        return 0;
    }

    player->gold -= price;
```

`inventory_add` returns `0` when `realloc` fails (Chapter 17). Charging
first and adding second would take the player's gold and give them
nothing on an out-of-memory — rare, but the kind of bug that only ever
shows up in a bug report you can't reproduce. Doing the *fallible* thing
first and the *infallible* thing second removes the possibility entirely.

This is a small, hand-rolled version of what databases call a
transaction. C gives you no rollback, so the discipline is to arrange
your code so nothing ever needs rolling back.

### Reporting failure without printing

Notice `shop_buy` takes a `char *reason` rather than calling `printf`:

```c
int shop_buy(const StockEntry *entry, Player *player, Inventory *inv,
             char *reason, size_t reason_size);
```

The function returns whether it worked, and writes *why* into a buffer
the caller owns. That keeps `shop.c` free of any knowledge about text
boxes, ncurses, or dialog paging — the same boundary Chapter 8 drew for
rendering and Chapter 15 drew for combat maths.

Passing `reason_size` alongside `reason` is the `snprintf` discipline from
Chapter 12 made into an interface: the function cannot know how big the
caller's buffer is unless it's told, and guessing is how buffer overflows
happen. Any function that writes into a caller's buffer should take its
size, every time.

### A tagged union, without the union

Shops sell two different kinds of thing — equipment and consumables —
which are stored in different tables and identified differently:

```c
typedef enum {
    STOCK_GEAR,
    STOCK_ITEM
} StockKind;

typedef struct {
    StockKind kind;
    GearSlot slot;      /* STOCK_GEAR only */
    int index;          /* gear index, or ItemId for STOCK_ITEM */
} StockEntry;
```

`kind` is the **tag**: it tells you which of the other fields mean
anything. For `STOCK_ITEM`, `slot` is ignored and `index` is an `ItemId`;
for `STOCK_GEAR`, both matter.

C has a `union` for exactly this — overlapping storage where only one
member is valid at a time — and this course deliberately doesn't use one
here. Two `int`s cost eight bytes; a union would save four and add a
layer of syntax for no practical gain at this scale. Knowing when *not*
to reach for a feature is worth as much as knowing the feature.

What matters is that the tag is checked before the fields are used:

```c
const char *shop_entry_name(const StockEntry *entry)
{
    if (entry == NULL) {
        return "(nothing)";
    }

    if (entry->kind == STOCK_GEAR) {
        const Gear *g = gear_at(entry->slot, entry->index);
        return (g != NULL) ? g->name : "(nothing)";
    }

    return item_type((ItemId)entry->index)->name;
}
```

Reading `index` as an `ItemId` when it's really a gear index would index
the wrong table entirely — and both are `int`, so nothing warns you. The
tag is the only thing standing between you and that bug.

## Apply it

### Shops as data, again

```c
static const Shop shops[] = {
    {   /* AREA_GRUBBIN_VALE */
        "Everything's twice what it was. That's the roads, not me.",
        {
            { STOCK_GEAR, SLOT_WEAPON,    1 },   /* Bronze Shortsword */
            { STOCK_GEAR, SLOT_ARMOUR,    1 },   /* Padded Jerkin     */
            { STOCK_GEAR, SLOT_ACCESSORY, 1 },   /* Lucky Sock        */
            { STOCK_ITEM, SLOT_WEAPON, ITEM_BREAD_RATION },
            { STOCK_ITEM, SLOT_WEAPON, ITEM_HERB_POULTICE }
        },
        5,
        6                                        /* inn: 6 gold */
    },
    {   /* AREA_WETWOOD_BARROW -- a barrow has no shopkeeper */
        NULL, { { 0, 0, 0 } }, 0, 0
    },
    ...
```

Indexed by area, exactly like the world table (Chapter 13), the enemy
rosters (Chapter 16), and the spell table (Chapter 18). By now this should
feel like the obvious way to express content — which is the point.

An area with no shop is expressed as a row of zeros rather than a special
case in code, and `shop_for_area` reports it honestly:

```c
if (shops[area_index].stock_count == 0 &&
    shops[area_index].inn_cost == 0) {
    return NULL;
}
```

### The inn

```c
int shop_rest(const Shop *shop, Player *player,
              char *reason, size_t reason_size)
{
    if (shop == NULL || shop->inn_cost <= 0) {
        snprintf(reason, reason_size, "There is no bed here.");
        return 0;
    }

    if (player->gold < shop->inn_cost) {
        snprintf(reason, reason_size, "A bed is %d. You have %d.",
                 shop->inn_cost, player->gold);
        return 0;
    }

    if (player->hp == player->max_hp && player->mp == player->max_mp) {
        snprintf(reason, reason_size, "You are not tired.");
        return 0;
    }

    player->gold -= shop->inn_cost;
    player->hp = player->max_hp;
    player->mp = player->max_mp;

    snprintf(reason, reason_size, "You sleep. HP and MP restored.");
    return 1;
}
```

Three refusals before any state changes. The third — "You are not tired."
— is the same courtesy as Chapter 17's "You are not hurt." for potions:
**never take payment for a no-op.** A player who pays six gold to sleep at
full health will assume the game cheated them, and they'll be right.

### Shopkeepers

An NPC gains one field:

```c
typedef struct {
    int x;
    int y;
    const char *speech;
    int shopkeeper;      /* 1 if talking opens the shop */
} Npc;
```

And walking into one says their piece *then* opens the stock list, using
the `dialog_return` mechanism Chapter 18 introduced:

```c
const Npc *npc = world_npc_at(game->world, target_x, target_y);
if (npc != NULL) {
    dialog_start(&game->dialog, npc->speech, TEXTBOX_WIDTH);

    /* A shopkeeper says their piece, then opens the stock list. */
    if (npc->shopkeeper &&
        shop_for_area((int)game->world->current) != NULL) {
        game->shop_index = 0;
        game->dialog_return = MODE_SHOP;
    } else {
        game->dialog_return = MODE_EXPLORE;
    }

    game->mode = MODE_DIALOG;
    return;
}
```

That `dialog_return` field existed for one chapter before finding its
second use. Worth noticing: the mechanism built to fix a bug (casting
mid-fight dumping you onto the map) turned out to be exactly the mechanism
this feature needed. That happens when a fix addresses the general problem
rather than the specific symptom.

### And the gear screen stops being a cheat

Chapter 18's gear screen cycled through every item in the tables for free.
Now that gear can be bought, that has to go:

```c
case INPUT_LEFT:
case INPUT_RIGHT:
    /* Gear is no longer free to swap: buying it in a shop is what
       equips it. The screen is now a read-only summary. */
    break;
```

`MODE_SHOP` joins the dispatch table as the seventh mode — one enum value,
one row, nothing else disturbed.

## Compile and run

```bash
make clean
make test
```

```
shop lookup
shop buying
inn
gold never goes negative

101163 checks, 0 failed
```

Then walk into Bess Harrow. She talks, and then she sells:

```
Grubbin Vale
 Gold 40
 > Bronze Shortsword    90
   Padded Jerkin        70
   Lucky Sock          120
```

With forty gold, the sword is refused — and you keep your forty:

```
Grubbin Vale
 Bronze Shortsword
 costs 90. You have
 40.
```

Scroll down to something affordable:

```
Grubbin Vale
 Gold 40
   Lucky Sock         120
 > Bread Ration        12
   Herb Poultice       40
```

```
Grubbin Vale
 Bread Ration
 bought. 28 gold
 left.
```

The inn is the last line, and refuses politely when you don't need it:

```
Grubbin Vale
 Gold 28
   Bread Ration        12
   Herb Poultice       40
 > A bed for the night    6
```

```
Grubbin Vale
 You are not tired.
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
  walk into a shopkeeper
        |
   dialog_start(greeting)            dialog_return = MODE_SHOP
        |
   [player reads, presses a key through the pages]
        |
   dialog_advance() returns 0  -->  mode = dialog_return = MODE_SHOP
        |
   shop_handle: cursor, then INPUT_CONFIRM
        |
   shop_buy()
     +-- entry NULL?          --> refuse, no change
     +-- gold < price?        --> refuse, no change
     +-- already wearing it?  --> refuse, no change
     +-- inventory_add fails? --> refuse, no change
     |
     +-- ALL CHECKS PASSED
           deduct gold
           equip gear / item now in bag
           write reason, return 1
```

Every arrow that leaves early leaves the player's gold and inventory
exactly as it found them. That's not a happy accident of how the `if`
statements fell — it's the direct result of putting every check above
every mutation, and it's why the fuzz test below can hammer the shop
thousands of times without ever producing negative gold.

## Common errors

**Charging before delivering:**

```c
player->gold -= price;
if (!inventory_add(inv, id, 1)) {
    return 0;        /* gold is gone, item never arrived */
}
```

Compiles, runs, and almost always works — `inventory_add` only fails when
`realloc` fails. That's exactly what makes it dangerous: untestable in
practice, wrong in principle, and impossible to reproduce from the bug
report. Order the operations so this can't happen rather than trying to
undo it.

**Forgetting the "already own it" check:**

```c
player->gold -= price;
player->equipped[entry->slot] = entry->index;
```

Nothing crashes. The player buys the sword they're already holding, gold
vanishes, and nothing about their character changes. They will assume the
purchase failed and try again.

**Reading a tagged struct's fields without checking the tag:**

```c
return item_type((ItemId)entry->index)->name;   /* even for STOCK_GEAR */
```

`index` is `1` for both the Bronze Shortsword and `ITEM_HERB_POULTICE`, so
this silently returns the wrong name — no crash, no warning, just a shop
selling "Herb Poultice" at 90 gold that turns out to be a sword. Both
fields are `int`; the type system cannot help here, only the tag can.

**Passing a buffer without its size:**

```c
int shop_buy(const StockEntry *entry, Player *player, Inventory *inv,
             char *reason);              /* how big is reason? */
```

The function has to guess, and any guess is wrong for some caller. Always
pass the size alongside the buffer.

## Exercises

1. Add selling. `CONTENT.md` says sell price is 50% of buy price
   throughout. Where does the "check everything, then commit" ordering
   apply differently when *removing* an item rather than adding one?
2. The Guild Signet gives a 10% shop discount. Implement it — where does
   the discount belong, `shop_entry_price` or `shop_buy`, and what breaks
   if you choose the wrong one?
3. `shop_rest` restores HP and MP but doesn't save the game. `CONTENT.md`
   describes inns as the save point, Dragon Warrior style. What would
   `shop_rest` need to know about in order to save, and why is that a
   reason to *not* put the save call inside it?
4. *Open-ended:* Prices are fixed forever. Sketch a scheme where prices
   respond to something — the area you're in, how far the wards have
   failed, what you've already bought. What would you need to store, and
   what would you have to be careful about when Chapter 20 starts saving
   the game?

<details>
<summary>Solutions</summary>

1. The ordering is the same principle but the fallible step swaps places.
   Adding: the *storage* can fail, so store first and charge second.
   Selling: the *removal* is what can fail (you might not own the item),
   so verify ownership and remove first, then pay:
   ```c
   if (!inventory_remove_one(inv, id)) {
       snprintf(reason, reason_size, "You do not have one.");
       return 0;
   }
   player->gold += shop_entry_price(entry) / 2;
   ```
   You'd also want to refuse key items outright — `item_type(id)->key_item`
   already marks them (Chapter 17), and selling the Brass Key would strand
   the player exactly as using it would have.

2. It belongs in `shop_entry_price`, so the discounted number is what the
   player *sees* in the list as well as what they're charged. Putting it
   in `shop_buy` means the display shows 120 and the purchase deducts 108
   — the player is pleasantly surprised rather than informed, and any
   "can I afford this?" reasoning they do from the screen is wrong. The
   general rule: apply price modifiers where the price is *computed*, not
   where it's *spent*, so display and behaviour cannot diverge.
   (`shop_entry_price` would then need access to the player, which is a
   signature change worth making deliberately.)

3. It would need the whole `Game` — the world, the player, the inventory,
   the RNG seed — because a save file contains all of it. That is a strong
   argument for *not* saving inside `shop_rest`: the function currently
   knows about exactly two things (`Shop` and `Player`) and is trivially
   testable because of it. Handing it the entire game state to enable one
   side effect would couple the shop module to everything. Better: let
   `shop_rest` return success, and have the *caller* — which already holds
   the `Game` — trigger the save. Same reasoning as `dialog.c` returning a
   value instead of changing the game mode itself (Chapter 12's exercise 3).

4. No fixed answer. A per-area price multiplier is the cheapest version
   and fits the existing `Shop` struct. Something dynamic — prices rising
   as the wards fail — means storing a world state number somewhere, and
   the Chapter 20 warning is this: **anything that affects prices must be
   in the save file, or a reload silently changes the economy.** More
   generally, every piece of mutable state you add from here on is
   something Chapter 20 has to serialise, which is a good reason to be
   deliberate about adding it. State that lives only in memory is state
   that disappears on load.

</details>

## Next up

Your hero has a level, spells, gear, a bag, and gold — and loses every
bit of it the moment you close the terminal. **Checkpoint E comes next**:
before Chapter 20 you'll decide how the game writes itself to disk, and
that choice determines whether you spend the chapter learning to parse
text or learning what struct padding does to a `fwrite`.
