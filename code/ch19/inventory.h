#ifndef INVENTORY_H
#define INVENTORY_H

typedef enum {
    ITEM_BREAD_RATION,
    ITEM_HERB_POULTICE,
    ITEM_BOTTLED_VIM,
    ITEM_BELL_OF_MILD_ALARM,
    ITEM_GUILD_COMMISSION,
    ITEM_BRASS_KEY,
    ITEM_TYPE_COUNT
} ItemId;

typedef enum {
    ITEM_EFFECT_NONE,
    ITEM_EFFECT_HEAL_HP,
    ITEM_EFFECT_HEAL_MP,
    ITEM_EFFECT_FLEE
} ItemEffect;

typedef struct {
    const char *name;
    ItemEffect effect;
    int magnitude;
    int key_item;     /* key items cannot be used or dropped */
    int cost;         /* shop price; 0 means not for sale */
} ItemType;

typedef struct {
    ItemId id;
    int quantity;
} ItemStack;

/* A dynamic array of stacks. Starts with no allocation at all; grows by
   doubling as items are added. */
typedef struct {
    ItemStack *stacks;
    int count;      /* stacks in use */
    int capacity;   /* stacks allocated */
} Inventory;

const ItemType *item_type(ItemId id);

void inventory_init(Inventory *inv);
void inventory_free(Inventory *inv);

/* Adds quantity of an item, stacking with an existing entry if present.
   Returns 0 if the inventory could not grow (out of memory). */
int inventory_add(Inventory *inv, ItemId id, int quantity);

/* Removes one of an item. Returns 0 if there was none to remove. */
int inventory_remove_one(Inventory *inv, ItemId id);

int inventory_count_of(const Inventory *inv, ItemId id);

#endif
