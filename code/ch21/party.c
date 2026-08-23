#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "party.h"

/* Indexed by level - 1. Straight from the content bible's level table.
   These numbers are unplaytested; expect to change them. */
static const LevelRow builtin_levels[PARTY_MAX_LEVEL] = {
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

/* The table actually in use. Starts as a copy of the built-in one, so
   the game works with no data files at all, and is replaced wholesale by
   a successful load. */
static LevelRow level_table[PARTY_MAX_LEVEL];
static int level_table_ready = 0;

static void ensure_table(void)
{
    if (!level_table_ready) {
        memcpy(level_table, builtin_levels, sizeof level_table);
        level_table_ready = 1;
    }
}

int party_load_levels(const char *path, char *reason, size_t reason_size)
{
    ensure_table();

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        snprintf(reason, reason_size, "no %s, using built-in levels", path);
        return 0;
    }

    /* Read into a scratch table; adopt it only if the whole file is
       good and every level is present. */
    LevelRow parsed[PARTY_MAX_LEVEL];
    int seen[PARTY_MAX_LEVEL];
    memset(seen, 0, sizeof seen);

    char line[256];
    int line_number = 0;
    int failed = 0;

    while (!failed && fgets(line, sizeof line, f) != NULL) {
        line_number++;

        char *p = line;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') {
            continue;
        }

        char keyword[16];
        int lv, hp, mp, atk, def, agi, xp;

        if (sscanf(p, "%15s %d %d %d %d %d %d %d",
                   keyword, &lv, &hp, &mp, &atk, &def, &agi, &xp) != 8 ||
            strcmp(keyword, "level") != 0) {
            snprintf(reason, reason_size, "%s:%d: expected a level line",
                     path, line_number);
            failed = 1;
            break;
        }

        if (lv < 1 || lv > PARTY_MAX_LEVEL) {
            snprintf(reason, reason_size, "%s:%d: level %d out of range",
                     path, line_number, lv);
            failed = 1;
            break;
        }

        if (hp < 1 || mp < 0 || atk < 0 || def < 0 || agi < 1 || xp < 0) {
            snprintf(reason, reason_size, "%s:%d: nonsensical stats",
                     path, line_number);
            failed = 1;
            break;
        }

        if (seen[lv - 1]) {
            snprintf(reason, reason_size, "%s:%d: level %d appears twice",
                     path, line_number, lv);
            failed = 1;
            break;
        }

        parsed[lv - 1].hp = hp;
        parsed[lv - 1].mp = mp;
        parsed[lv - 1].attack = atk;
        parsed[lv - 1].defense = def;
        parsed[lv - 1].agility = agi;
        parsed[lv - 1].xp_to_next = xp;
        seen[lv - 1] = 1;
    }

    fclose(f);

    if (!failed) {
        for (int i = 0; i < PARTY_MAX_LEVEL; i++) {
            if (!seen[i]) {
                snprintf(reason, reason_size, "%s: level %d is missing",
                         path, i + 1);
                failed = 1;
                break;
            }
        }
    }

    /* The cap must terminate progression or party_gain_xp would spin. */
    if (!failed && parsed[PARTY_MAX_LEVEL - 1].xp_to_next != 0) {
        snprintf(reason, reason_size, "%s: level %d must have xp_to_next 0",
                 path, PARTY_MAX_LEVEL);
        failed = 1;
    }

    if (failed) {
        return 0;
    }

    memcpy(level_table, parsed, sizeof level_table);
    snprintf(reason, reason_size, "loaded %s", path);
    return 1;
}

const LevelRow *party_level_row(int level)
{
    if (level < 1) {
        level = 1;
    }
    if (level > PARTY_MAX_LEVEL) {
        level = PARTY_MAX_LEVEL;
    }

    ensure_table();
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
