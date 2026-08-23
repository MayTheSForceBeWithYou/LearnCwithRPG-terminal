#ifndef PARTY_H
#define PARTY_H

#include <stddef.h>
#include "entity.h"

/* Progression for a solo hero (Checkpoint B). The table is transcribed
   from the content bible and is a STARTING POINT -- it has not been
   playtested and almost certainly needs tuning. */

#define PARTY_MAX_LEVEL 20

typedef struct {
    int hp;
    int mp;
    int attack;
    int defense;
    int agility;
    int xp_to_next;   /* XP needed to leave this level; 0 at the cap */
} LevelRow;

#define PARTY_LEVELS_PATH "assets/levels.txt"

/* Replaces the built-in table with one read from disk. Returns 1 if the
   file was loaded; returns 0 and leaves the built-in table in place if
   the file is missing or malformed, writing the reason into reason. A
   partially-read file is never adopted. */
int party_load_levels(const char *path, char *reason, size_t reason_size);

const LevelRow *party_level_row(int level);

/* Applies the stats for the hero's current level. Used on level-up and
   when first creating the hero. */
void party_apply_level(Player *p);

/* Adds xp and levels the hero up as many times as the total allows.
   Returns the number of levels gained (0 if none). */
int party_gain_xp(Player *p, int xp);

#endif
