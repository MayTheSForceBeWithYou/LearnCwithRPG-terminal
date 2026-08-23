#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "inventory.h"

/* Item definitions, from the content bible. Indexed by ItemId. */
static const ItemType item_types[ITEM_TYPE_COUNT] = {
    [ITEM_BREAD_RATION]      = { "Bread Ration",       ITEM_EFFECT_HEAL_HP,  25, 0,  12 },
    [ITEM_HERB_POULTICE]     = { "Herb Poultice",      ITEM_EFFECT_HEAL_HP,  60, 0,  40 },
    [ITEM_BOTTLED_VIM]       = { "Bottled Vim",        ITEM_EFFECT_HEAL_MP,  20, 0,  90 },
    [ITEM_BELL_OF_MILD_ALARM]= { "Bell of Mild Alarm", ITEM_EFFECT_FLEE,      0, 0,  60 },
    [ITEM_GUILD_COMMISSION]  = { "Guild Commission",   ITEM_EFFECT_NONE,      0, 1,   0 },
    [ITEM_BRASS_KEY]         = { "Brass Key",          ITEM_EFFECT_NONE,      0, 1,   0 }
};

#define INVENTORY_INITIAL_CAPACITY 4

const ItemType *item_type(ItemId id)
{
    if (id < 0 || id >= ITEM_TYPE_COUNT) {
        return &item_types[0];
    }

    return &item_types[id];
}

void inventory_init(Inventory *inv)
{
    /* No allocation until the first item arrives. A hero who never picks
       anything up never costs a byte of heap. */
    inv->stacks = NULL;
    inv->count = 0;
    inv->capacity = 0;
}

void inventory_free(Inventory *inv)
{
    free(inv->stacks);
    inv->stacks = NULL;
    inv->count = 0;
    inv->capacity = 0;
}

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

static ItemStack *find_stack(Inventory *inv, ItemId id)
{
    for (int i = 0; i < inv->count; i++) {
        if (inv->stacks[i].id == id) {
            return &inv->stacks[i];
        }
    }

    return NULL;
}

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

    /* Growth must have produced room, or the write below is out of
       bounds. This is the invariant inventory_grow exists to maintain. */
    assert(inv->count < inv->capacity);
    assert(inv->stacks != NULL);

    inv->stacks[inv->count].id = id;
    inv->stacks[inv->count].quantity = quantity;
    inv->count++;

    return 1;
}

int inventory_remove_one(Inventory *inv, ItemId id)
{
    ItemStack *stack = find_stack(inv, id);
    if (stack == NULL || stack->quantity <= 0) {
        return 0;
    }

    stack->quantity--;

    if (stack->quantity == 0) {
        /* Close the gap by moving the last stack into this slot. Order
           is not meaningful here, so this beats shuffling everything. */
        int index = (int)(stack - inv->stacks);
        inv->stacks[index] = inv->stacks[inv->count - 1];
        inv->count--;
    }

    return 1;
}

int inventory_count_of(const Inventory *inv, ItemId id)
{
    for (int i = 0; i < inv->count; i++) {
        if (inv->stacks[i].id == id) {
            return inv->stacks[i].quantity;
        }
    }

    return 0;
}
