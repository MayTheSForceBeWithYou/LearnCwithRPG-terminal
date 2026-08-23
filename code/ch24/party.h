#ifndef PARTY_H
#define PARTY_H

#include <stddef.h>
#include "entity.h"

/* Progression for a solo hero (Checkpoint B). The table is transcribed
   from the content bible and is a STARTING POINT -- it has not been
   playtested and almost certainly needs tuning. */

#define PARTY_MAX_LEVEL 20

/* Ceilings for the two values that accumulate without bound. Adding to
   an int that is already near INT_MAX is undefined behaviour, not a
   harmless wrap -- so both are clamped rather than allowed to overflow.
   Chapter 22 found this with UBSan. */
#define PARTY_MAX_XP   1000000
#define PARTY_MAX_GOLD 1000000

typedef struct {
    int hp;
    int mp;
    int attack;
    int defense;
    int agility;
    int xp_to_next;   /* XP needed to leave this level; 0 at the cap */
} LevelRow;

/* Relative to the data directory -- see paths.h. */
#define PARTY_LEVELS_FILE "levels.txt"

/* Replaces the built-in table with one read from disk. Returns 1 if the
   file was loaded; returns 0 and leaves the built-in table in place if
   the file is missing or malformed, writing the reason into reason. A
   partially-read file is never adopted. */
int party_load_levels(const char *path, char *reason, size_t reason_size);

const LevelRow *party_level_row(int level);

/* Applies the stats for the hero's current level. Used on level-up and
   when first creating the hero.

   Requires p->hp and p->mp to already hold meaningful values, because it
   clamps them against the new maximums. Passing a Player whose hp/mp
   have never been written is undefined behaviour -- zero the struct
   first. */
void party_apply_level(Player *p);

/* Adds xp and levels the hero up as many times as the total allows.
   Returns the number of levels gained (0 if none). Total XP is clamped
   to PARTY_MAX_XP, so this can never overflow. */
int party_gain_xp(Player *p, int xp);

/* Adds gold, clamped to PARTY_MAX_GOLD. Use this rather than += so the
   ceiling is enforced in exactly one place. */
void party_add_gold(Player *p, int amount);

#endif
