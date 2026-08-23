#ifndef ENTITY_H
#define ENTITY_H

typedef enum {
    RANK_E,
    RANK_D,
    RANK_C,
    RANK_B,
    RANK_A,
    RANK_S
} Rank;

/* Equipment, from the content bible. Slot 0 of each table is the
   starting gear, so a fresh hero is never empty-handed. */
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

int gear_count(GearSlot slot);
const Gear *gear_at(GearSlot slot, int index);

typedef struct {
    char name[64];
    int x;
    int y;
    int hp;
    int max_hp;
    int mp;
    int max_mp;
    int attack;
    int defense;
    int agility;
    int level;
    int xp;
    int gold;
    unsigned status;
    int status_turns;
    int equipped[SLOT_COUNT];   /* index into each slot's gear table */
    Rank rank;
} Player;

/* Base stat plus whatever is equipped. Everything that reads a stat for
   combat must go through these, or gear silently does nothing. */
int entity_attack(const Player *p);
int entity_defense(const Player *p);
int entity_agility(const Player *p);

typedef struct {
    int x;
    int y;
    const char *speech;
    int shopkeeper;      /* 1 if talking opens the shop */
} Npc;

Player entity_create_player(void);
void entity_print_sheet(const Player *p);
void entity_move(Player *p, int dx, int dy);

#endif
