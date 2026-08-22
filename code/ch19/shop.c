#include <stdio.h>
#include <stddef.h>
#include "shop.h"

/* Stock lists from the content bible, one per area. Index order matches
   AreaId, the same convention as the world and enemy tables. */
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
    {   /* AREA_CASTLE_HOLLIS */
        "The Guild's credit is good here. Yours is a separate matter.",
        {
            { STOCK_GEAR, SLOT_WEAPON,    2 },   /* Woodsman's Axe */
            { STOCK_GEAR, SLOT_ARMOUR,    2 },   /* Boiled Leather */
            { STOCK_GEAR, SLOT_ACCESSORY, 2 },   /* Ledger-Weight  */
            { STOCK_ITEM, SLOT_WEAPON, ITEM_BOTTLED_VIM },
            { STOCK_ITEM, SLOT_WEAPON, ITEM_BELL_OF_MILD_ALARM }
        },
        5,
        90                                       /* inn: 90 gold */
    }
};

#define SHOP_COUNT ((int)(sizeof shops / sizeof shops[0]))

const Shop *shop_for_area(int area_index)
{
    if (area_index < 0 || area_index >= SHOP_COUNT) {
        return NULL;
    }

    if (shops[area_index].stock_count == 0 &&
        shops[area_index].inn_cost == 0) {
        return NULL;
    }

    return &shops[area_index];
}

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

int shop_entry_price(const StockEntry *entry)
{
    if (entry == NULL) {
        return 0;
    }

    if (entry->kind == STOCK_GEAR) {
        const Gear *g = gear_at(entry->slot, entry->index);
        return (g != NULL) ? g->cost : 0;
    }

    return item_type((ItemId)entry->index)->cost;
}

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

    if (entry->kind == STOCK_GEAR) {
        /* Refuse rather than take money for something already worn. */
        if (player->equipped[entry->slot] == entry->index) {
            snprintf(reason, reason_size, "You are already wearing that.");
            return 0;
        }

        player->gold -= price;
        player->equipped[entry->slot] = entry->index;

        snprintf(reason, reason_size, "%s equipped. %d gold left.",
                 name, player->gold);
        return 1;
    }

    /* Consumables can fail to be stored if the inventory cannot grow, so
       take the money only once the item is safely in the bag. */
    if (!inventory_add(inv, (ItemId)entry->index, 1)) {
        snprintf(reason, reason_size, "You cannot carry any more.");
        return 0;
    }

    player->gold -= price;

    snprintf(reason, reason_size, "%s bought. %d gold left.",
             name, player->gold);
    return 1;
}

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
