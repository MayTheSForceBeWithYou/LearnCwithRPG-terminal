#include <stdio.h>
#include <string.h>
#include "battle.h"
#include "combat_math.h"
#include "party.h"

/* Enemy rosters, from the content bible. One roster per area, indexed
   the same way AreaId is -- another lookup table. Unplaytested. */
static const EnemyType grubbin_vale_enemies[] = {
    { "Disgruntled Slime",  14,  5, 1,  3, 3,  5 },
    { "Turnip Blight",      18,  6, 2,  4, 4,  6 },
    { "Field Rat",          12,  7, 1,  8, 4,  4 },
    { "Committee of Bats",  22,  6, 2, 11, 6,  9 }
};

static const EnemyType wetwood_barrow_enemies[] = {
    { "Barrow Moth",        30, 11, 4, 12, 12, 14 },
    { "Grave-Damp",         44, 13, 7,  5, 16, 20 },
    { "Unlicensed Skeleton",38, 15, 6,  9, 18, 22 },
    { "Mourner",            52, 14, 9,  7, 22, 30 }
};

typedef struct {
    const EnemyType *types;
    int count;
    int max_group;
} Roster;

static const Roster rosters[] = {
    { grubbin_vale_enemies,
      (int)(sizeof grubbin_vale_enemies / sizeof grubbin_vale_enemies[0]), 3 },
    { wetwood_barrow_enemies,
      (int)(sizeof wetwood_barrow_enemies / sizeof wetwood_barrow_enemies[0]), 2 },
    /* Castle Hollis is a safe hub: no roster, no encounters. */
    { NULL, 0, 0 }
};

#define ROSTER_COUNT ((int)(sizeof rosters / sizeof rosters[0]))

static void log_line(Battle *battle, const char *text)
{
    if (battle->log_count >= BATTLE_LOG_LINES) {
        /* Scroll: drop the oldest line and shuffle the rest up. */
        for (int i = 1; i < BATTLE_LOG_LINES; i++) {
            memcpy(battle->log[i - 1], battle->log[i], BATTLE_LOG_LEN);
        }
        battle->log_count = BATTLE_LOG_LINES - 1;
    }

    snprintf(battle->log[battle->log_count], BATTLE_LOG_LEN, "%s", text);
    battle->log_count++;
}

int battle_max_group_for_level(int hero_level)
{
    if (hero_level < 3) {
        return 1;
    }
    if (hero_level < 6) {
        return 2;
    }
    return 3;
}

void battle_begin(Battle *battle, Rng *rng, int area_index, int hero_level)
{
    memset(battle, 0, sizeof *battle);

    if (area_index < 0 || area_index >= ROSTER_COUNT ||
        rosters[area_index].types == NULL) {
        /* No roster for this area -- produce an already-won battle
           rather than an empty one nobody can leave. */
        battle->outcome = BATTLE_WON;
        return;
    }

    const Roster *roster = &rosters[area_index];

    int cap = battle_max_group_for_level(hero_level);
    if (cap > roster->max_group) {
        cap = roster->max_group;
    }

    int count = rng_range(rng, 1, cap);

    for (int i = 0; i < count; i++) {
        const EnemyType *type =
            &roster->types[rng_range(rng, 0, roster->count - 1)];

        battle->enemies[i].type = type;
        battle->enemies[i].hp = type->max_hp;
        battle->enemies[i].alive = 1;
    }

    battle->enemy_count = count;
    battle->target = 0;
    battle->outcome = BATTLE_ONGOING;

    char line[BATTLE_LOG_LEN];
    if (count == 1) {
        snprintf(line, sizeof line, "A %s blocks the way.",
                 battle->enemies[0].type->name);
    } else {
        snprintf(line, sizeof line, "%d enemies block the way.", count);
    }
    log_line(battle, line);
}

int battle_living_enemies(const Battle *battle)
{
    int living = 0;

    for (int i = 0; i < battle->enemy_count; i++) {
        if (battle->enemies[i].alive) {
            living++;
        }
    }

    return living;
}

void battle_cycle_target(Battle *battle, int direction)
{
    if (battle_living_enemies(battle) == 0) {
        return;
    }

    int index = battle->target;

    for (int guard = 0; guard < BATTLE_MAX_ENEMIES + 1; guard++) {
        index += direction;

        if (index < 0) {
            index = battle->enemy_count - 1;
        }
        if (index >= battle->enemy_count) {
            index = 0;
        }

        if (battle->enemies[index].alive) {
            battle->target = index;
            return;
        }
    }
}

/* Make sure the cursor is not sitting on a corpse. */
static void ensure_valid_target(Battle *battle)
{
    if (battle->target >= 0 && battle->target < battle->enemy_count &&
        battle->enemies[battle->target].alive) {
        return;
    }

    for (int i = 0; i < battle->enemy_count; i++) {
        if (battle->enemies[i].alive) {
            battle->target = i;
            return;
        }
    }
}

static void hero_attacks(Battle *battle, Player *player, Rng *rng)
{
    ensure_valid_target(battle);

    Enemy *enemy = &battle->enemies[battle->target];
    if (!enemy->alive) {
        return;
    }

    char line[BATTLE_LOG_LEN];

    if (!combat_roll_hit(rng, player->agility, enemy->type->agility)) {
        snprintf(line, sizeof line, "You swing at the %s and miss.",
                 enemy->type->name);
        log_line(battle, line);
        return;
    }

    int critical = 0;
    int damage = combat_roll_damage(rng, player->attack, 0,
                                    enemy->type->defense, &critical);

    enemy->hp -= damage;

    snprintf(line, sizeof line, "%s the %s for %d.",
             critical ? "CRITICAL on" : "You hit", enemy->type->name, damage);
    log_line(battle, line);

    if (enemy->hp <= 0) {
        enemy->hp = 0;
        enemy->alive = 0;

        battle->xp_reward += enemy->type->xp;
        battle->gold_reward += enemy->type->gold;

        snprintf(line, sizeof line, "The %s falls.", enemy->type->name);
        log_line(battle, line);
    }
}

static void enemy_attacks(Battle *battle, Player *player, Rng *rng,
                          const Enemy *enemy, int enemy_damage_pct)
{
    char line[BATTLE_LOG_LEN];

    if (!combat_roll_hit(rng, enemy->type->agility, player->agility)) {
        snprintf(line, sizeof line, "The %s misses you.", enemy->type->name);
        log_line(battle, line);
        return;
    }

    int critical = 0;
    int damage = combat_roll_damage(rng, enemy->type->attack, 0,
                                    player->defense, &critical);

    /* Apply the difficulty multiplier, never letting it round a real
       hit down to nothing. */
    damage = (damage * enemy_damage_pct) / 100;
    if (damage < 1) {
        damage = 1;
    }

    player->hp -= damage;
    if (player->hp < 0) {
        player->hp = 0;
    }

    snprintf(line, sizeof line, "The %s hits you for %d.%s",
             enemy->type->name, damage, critical ? " Critical." : "");
    log_line(battle, line);
}

void battle_round(Battle *battle, Player *player, Rng *rng, int fleeing,
                  int enemy_damage_pct)
{
    if (battle->outcome != BATTLE_ONGOING) {
        return;
    }

    /* Turn order: everyone sorted by agility, highest first, ties to the
       hero (Checkpoint C). With a solo hero the "sort" is just deciding
       which enemies act before the hero and which after. */
    if (fleeing) {
        /* Fleeing is measured against the fastest living enemy. */
        int fastest = 0;
        for (int i = 0; i < battle->enemy_count; i++) {
            if (battle->enemies[i].alive &&
                battle->enemies[i].type->agility > fastest) {
                fastest = battle->enemies[i].type->agility;
            }
        }

        int chance = combat_flee_chance(player->agility, fastest);
        if (rng_range(rng, 1, 100) <= chance) {
            log_line(battle, "You break away and run.");
            battle->outcome = BATTLE_FLED;
            return;
        }

        log_line(battle, "You try to run, and fail.");
    }

    /* Enemies faster than the hero strike first. */
    for (int i = 0; i < battle->enemy_count; i++) {
        const Enemy *enemy = &battle->enemies[i];
        if (enemy->alive && enemy->type->agility > player->agility) {
            enemy_attacks(battle, player, rng, enemy, enemy_damage_pct);

            if (player->hp <= 0) {
                battle->outcome = BATTLE_LOST;
                return;
            }
        }
    }

    if (!fleeing) {
        hero_attacks(battle, player, rng);

        if (battle_living_enemies(battle) == 0) {
            battle->outcome = BATTLE_WON;
            return;
        }
    }

    /* Then everyone the hero outpaced (ties included -- hero wins ties). */
    for (int i = 0; i < battle->enemy_count; i++) {
        const Enemy *enemy = &battle->enemies[i];
        if (enemy->alive && enemy->type->agility <= player->agility) {
            enemy_attacks(battle, player, rng, enemy, enemy_damage_pct);

            if (player->hp <= 0) {
                battle->outcome = BATTLE_LOST;
                return;
            }
        }
    }

    ensure_valid_target(battle);
}
