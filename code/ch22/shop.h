#ifndef SHOP_H
#define SHOP_H

#include "entity.h"
#include "inventory.h"

/* What a shop sells. Gear is referenced by slot and index into the gear
   tables; consumables by ItemId. One row per line of stock. */
typedef enum {
    STOCK_GEAR,
    STOCK_ITEM
} StockKind;

typedef struct {
    StockKind kind;
    GearSlot slot;      /* STOCK_GEAR only */
    int index;          /* gear index, or ItemId for STOCK_ITEM */
} StockEntry;

#define SHOP_MAX_STOCK 6

typedef struct {
    const char *greeting;
    StockEntry stock[SHOP_MAX_STOCK];
    int stock_count;
    int inn_cost;       /* 0 if this shop has no bed */
} Shop;

/* Shops are per-area, indexed like AreaId. Returns NULL where an area
   has no shop. */
const Shop *shop_for_area(int area_index);

const char *shop_entry_name(const StockEntry *entry);
int shop_entry_price(const StockEntry *entry);

/* Sells to the hero. Returns 1 on success; on failure returns 0 and
   writes the reason into reason (which must hold at least 48 bytes). */
int shop_buy(const StockEntry *entry, Player *player, Inventory *inv,
             char *reason, size_t reason_size);

/* Charges for a bed and restores HP/MP. Same return convention. */
int shop_rest(const Shop *shop, Player *player,
              char *reason, size_t reason_size);

#endif
