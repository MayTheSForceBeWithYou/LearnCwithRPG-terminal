#include <stddef.h>
#include "combat_math.h"

static int clamp(int value, int low, int high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

int combat_base_damage(int attack, int weapon_power, int defense)
{
    int offence = attack + weapon_power;
    int base = offence - (defense / 2);

    /* Even a hopeless attack scratches for 1, so a fight can always
       eventually end rather than deadlocking at zero damage. */
    if (base < 1) {
        base = 1;
    }

    return base;
}

int combat_critical_damage(int attack, int weapon_power)
{
    int base = attack + weapon_power;

    if (base < 1) {
        base = 1;
    }

    return base * 2;
}

int combat_apply_variance(int base, int roll)
{
    /* roll 0 -> -12%, roll 12 -> 0%, roll 24 -> +12%. */
    int offset = roll - COMBAT_VARIANCE_PERCENT;

    /* Multiply before dividing so the percentage does not vanish to
       zero under integer division for small base values. */
    int delta = (base * offset) / 100;

    int result = base + delta;
    if (result < 1) {
        result = 1;
    }

    return result;
}

int combat_hit_chance(int attacker_agi, int defender_agi)
{
    return clamp(85 + (attacker_agi - defender_agi), 10, 99);
}

int combat_flee_chance(int hero_agi, int enemy_agi)
{
    return clamp(50 + (hero_agi - enemy_agi) * 2, 10, 90);
}

int combat_roll_damage(Rng *rng, int attack, int weapon_power, int defense,
                       int *out_was_critical)
{
    int critical = (rng_range(rng, 1, COMBAT_CRIT_IN_N) == 1);

    if (out_was_critical != NULL) {
        *out_was_critical = critical;
    }

    int base = critical
             ? combat_critical_damage(attack, weapon_power)
             : combat_base_damage(attack, weapon_power, defense);

    int roll = rng_range(rng, 0, 2 * COMBAT_VARIANCE_PERCENT);

    return combat_apply_variance(base, roll);
}

int combat_roll_hit(Rng *rng, int attacker_agi, int defender_agi)
{
    int chance = combat_hit_chance(attacker_agi, defender_agi);
    return rng_range(rng, 1, 100) <= chance;
}
