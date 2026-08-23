#include <stdio.h>
#include <string.h>
#include "entity.h"
#include "party.h"
#include "magic.h"

static char rank_to_char(Rank rank)
{
    switch (rank) {
        case RANK_E: return 'E';
        case RANK_D: return 'D';
        case RANK_C: return 'C';
        case RANK_B: return 'B';
        case RANK_A: return 'A';
        case RANK_S: return 'S';
    }
    return '?';
}


/* ---- equipment tables ---- */

static const Gear weapons[] = {
    { "Chair Leg",             2, 0, 0,    0 },
    { "Bronze Shortsword",     6, 0, 0,   90 },
    { "Woodsman's Axe",       11, 0, 0,  260 },
    { "Guild-Issue Longsword",17, 0, 0,  620 },
    { "Silver Rapier",        24, 0, 0, 1400 }
};

static const Gear armour[] = {
    { "Work Clothes",   0,  1, 0,    0 },
    { "Padded Jerkin",  0,  4, 0,   70 },
    { "Boiled Leather", 0,  8, 0,  230 },
    { "Ring Mail",      0, 13, 0,  580 },
    { "Guild Plate",    0, 19, 0, 1500 }
};

static const Gear accessories[] = {
    { "(none)",        0, 0,  0,    0 },
    { "Lucky Sock",    0, 0,  2,  120 },
    { "Ledger-Weight", 0, 5, -2,  650 }
};

static const Gear *const gear_tables[SLOT_COUNT] = {
    weapons, armour, accessories
};

static const int gear_counts[SLOT_COUNT] = {
    (int)(sizeof weapons / sizeof weapons[0]),
    (int)(sizeof armour / sizeof armour[0]),
    (int)(sizeof accessories / sizeof accessories[0])
};

int gear_count(GearSlot slot)
{
    if (slot < 0 || slot >= SLOT_COUNT) {
        return 0;
    }

    return gear_counts[slot];
}

const Gear *gear_at(GearSlot slot, int index)
{
    if (slot < 0 || slot >= SLOT_COUNT ||
        index < 0 || index >= gear_counts[slot]) {
        return NULL;
    }

    return &gear_tables[slot][index];
}

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

int entity_defense(const Player *p)
{
    int total = p->defense;

    for (int s = 0; s < SLOT_COUNT; s++) {
        const Gear *g = gear_at((GearSlot)s, p->equipped[s]);
        if (g != NULL) {
            total += g->defense;
        }
    }

    /* Stoneskin is a percentage on top of everything else. */
    if (p->status & STATUS_STONESKIN) {
        total += total * 40 / 100;
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

Player entity_create_player(void)
{
    /* Zero the whole struct before anything reads it. party_apply_level
       clamps hp against max_hp, so it reads hp -- and reading an
       uninitialised local is undefined behaviour, not merely untidy.
       Valgrind found this in Chapter 22; ASan cannot see it. */
    Player p;
    memset(&p, 0, sizeof p);

    printf("What is your name, hero? ");
    if (fgets(p.name, sizeof p.name, stdin) == NULL) {
        p.name[0] = '\0';
    }

    /* fgets keeps the newline. Trim it here, once, so every later use of
       the name is a plain string -- printing it, saving it, loading it
       back. Relying on that trailing newline for formatting broke the
       moment a name came from a save file instead of the keyboard. */
    size_t len = strlen(p.name);
    while (len > 0 && (p.name[len - 1] == '\n' || p.name[len - 1] == '\r')) {
        p.name[len - 1] = '\0';
        len--;
    }

    if (p.name[0] == '\0') {
        snprintf(p.name, sizeof p.name, "Wick");
    }

    p.x = 1;
    p.y = 1;
    p.level = 1;
    p.status = STATUS_NONE;
    p.status_turns = 0;
    p.equipped[SLOT_WEAPON] = 0;      /* Chair Leg    */
    p.equipped[SLOT_ARMOUR] = 0;      /* Work Clothes */
    p.equipped[SLOT_ACCESSORY] = 0;   /* nothing      */
    p.xp = 0;
    p.gold = 40;      /* the Guild's advance, such as it is */
    p.rank = RANK_E;

    /* Stats come from the level table rather than being written here,
       so there is exactly one place they are defined. */
    party_apply_level(&p);
    p.hp = p.max_hp;
    p.mp = p.max_mp;

    return p;
}

void entity_print_sheet(const Player *p)
{
    /* Effective stats, so equipment shows up here as well as in battle. */
    int atk = entity_attack(p);
    int def = entity_defense(p);
    float power_rating = (float)(atk + def) / 2.0f;

    printf("\n");
    printf("Welcome, %s\n", p->name);
    printf("----------------------------------------\n");
    printf("Level:  %d\n", p->level);
    printf("HP:     %d/%d\n", p->hp, p->max_hp);
    printf("MP:     %d/%d\n", p->mp, p->max_mp);
    printf("Gold:   %d\n", p->gold);
    printf("ATK:    %d\n", atk);
    printf("DEF:    %d\n", def);
    printf("Rank:   %c\n", rank_to_char(p->rank));
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}

void entity_move(Player *p, int dx, int dy)
{
    p->x += dx;
    p->y += dy;
}
