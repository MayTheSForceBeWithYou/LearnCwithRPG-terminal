#ifndef COMBAT_MATH_H
#define COMBAT_MATH_H

#include "rng.h"

/* Damage numbers you can trust, per Checkpoint C.

   Everything above the line is a PURE function: same arguments in, same
   answer out, no randomness and no state. That is what makes them
   testable. The two functions below the line take an Rng * and are the
   only ones that can surprise you. */

#define COMBAT_VARIANCE_PERCENT 12
#define COMBAT_CRIT_IN_N        32

/* base = (attack + weapon_power) - (defense / 2), never below 1. */
int combat_base_damage(int attack, int weapon_power, int defense);

/* A critical hit doubles the damage and ignores defence entirely. */
int combat_critical_damage(int attack, int weapon_power);

/* roll is a variance step in [0, 2 * COMBAT_VARIANCE_PERCENT], where 0
   means -12%, 12 means no change, and 24 means +12%. Result is never
   below 1. Kept separate from the RNG so it can be tested exhaustively. */
int combat_apply_variance(int base, int roll);

/* 85 + (attacker_agi - defender_agi), clamped to [10, 99]. */
int combat_hit_chance(int attacker_agi, int defender_agi);

/* 50 + (hero_agi - enemy_agi) * 2, clamped to [10, 90]. */
int combat_flee_chance(int hero_agi, int enemy_agi);

/* ---- the two that consume randomness ---- */

int combat_roll_damage(Rng *rng, int attack, int weapon_power, int defense,
                       int *out_was_critical);

int combat_roll_hit(Rng *rng, int attacker_agi, int defender_agi);

#endif
