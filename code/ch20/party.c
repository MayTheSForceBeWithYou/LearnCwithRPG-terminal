#include "party.h"

/* Indexed by level - 1. Straight from the content bible's level table.
   These numbers are unplaytested; expect to change them. */
static const LevelRow level_table[PARTY_MAX_LEVEL] = {
    /*  HP   MP  ATK  DEF  AGI   XP to next */
    {   20,   0,   5,   3,   5,      8 },   /* level 1  */
    {   26,   2,   6,   4,   6,     18 },
    {   32,   4,   8,   5,   6,     32 },
    {   39,   6,   9,   6,   7,     55 },
    {   47,   9,  11,   8,   8,     90 },
    {   55,  12,  13,   9,   9,    140 },
    {   64,  15,  15,  11,  10,    210 },
    {   73,  18,  17,  12,  11,    300 },
    {   83,  22,  19,  14,  12,    420 },
    {   94,  26,  21,  16,  13,    580 },   /* level 10 */
    {  105,  30,  24,  18,  14,    780 },
    {  117,  35,  26,  20,  15,   1030 },
    {  130,  40,  29,  22,  17,   1350 },
    {  143,  45,  32,  24,  18,   1750 },
    {  157,  51,  35,  27,  19,   2250 },
    {  172,  57,  38,  29,  21,   2850 },
    {  188,  63,  41,  32,  22,   3600 },
    {  205,  70,  45,  35,  24,   4500 },
    {  222,  77,  48,  38,  25,   5600 },
    {  240,  85,  52,  41,  27,      0 }    /* level 20, the cap */
};

const LevelRow *party_level_row(int level)
{
    if (level < 1) {
        level = 1;
    }
    if (level > PARTY_MAX_LEVEL) {
        level = PARTY_MAX_LEVEL;
    }

    return &level_table[level - 1];
}

void party_apply_level(Player *p)
{
    const LevelRow *row = party_level_row(p->level);

    p->max_hp = row->hp;
    p->max_mp = row->mp;
    p->attack = row->attack;
    p->defense = row->defense;
    p->agility = row->agility;

    if (p->hp > p->max_hp) {
        p->hp = p->max_hp;
    }
    if (p->mp > p->max_mp) {
        p->mp = p->max_mp;
    }
}

int party_gain_xp(Player *p, int xp)
{
    if (xp <= 0) {
        return 0;
    }

    p->xp += xp;

    int levels_gained = 0;

    while (p->level < PARTY_MAX_LEVEL) {
        const LevelRow *row = party_level_row(p->level);

        if (row->xp_to_next <= 0 || p->xp < row->xp_to_next) {
            break;
        }

        p->xp -= row->xp_to_next;
        p->level++;
        levels_gained++;

        /* A level-up restores the hero to full, which is the classic
           Dragon Warrior behaviour and a small mercy on a long dungeon. */
        party_apply_level(p);
        p->hp = p->max_hp;
        p->mp = p->max_mp;
    }

    return levels_gained;
}
