#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "battle.h"
#include "inventory.h"
#include "combat_math.h"
#include "party.h"
#include "magic.h"

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

static const EnemyType the_sump_enemies[] = {
    { "Sump Leech",          60, 20,  8, 14, 34, 40 },
    { "Bog Hulk",           105, 26, 16,  6, 55, 65 },
    { "Marsh Lantern",       70, 24, 10, 16, 46, 58 },
    { "Drowned Clerk",       88, 28, 13, 11, 60, 80 }
};

static const Roster rosters[] = {
    { grubbin_vale_enemies,
      (int)(sizeof grubbin_vale_enemies / sizeof grubbin_vale_enemies[0]), 3 },
    { wetwood_barrow_enemies,
      (int)(sizeof wetwood_barrow_enemies / sizeof wetwood_barrow_enemies[0]), 2 },
    /* Castle Hollis is a safe hub: no roster, no encounters. */
    { NULL, 0, 0 },
    /* Mudwick is a town -- also safe. */
    { NULL, 0, 0 },
    { the_sump_enemies,
      (int)(sizeof the_sump_enemies / sizeof the_sump_enemies[0]), 3 },
    /* The Crownless Court holds one thing, and it is not a random
       encounter. */
    { NULL, 0, 0 }
};

#define ROSTER_COUNT ((int)(sizeof rosters / sizeof rosters[0]))

/* The bosses. Stats from the content bible, scaled down with the level
   curve now capping near 15 rather than 20 (Checkpoint F). Still
   unplaytested -- expect to retune these.

   Each carries the three things it says as the fight turns against it.
   Phase lines are content, so they live in the table beside the stats
   rather than in a switch somewhere in the combat code. */
static const BossType boss_table[BOSS_COUNT] = {
    [BOSS_BOGWRIGHT] = {
        .type = { "Bogwright", 180, 18, 10, 6, 120, 150 },
        .phase_lines = {
            "The Bogwright apologises, and swings.",
            "\"I was asked to. Nobody came back.\"",
            "\"Then I may stop. Thank you.\""
        },
        .reward_item = ITEM_FRAGMENT_I
    },
    [BOSS_SUMP_AUDITOR] = {
        .type = { "The Sump Auditor", 420, 32, 18, 12, 400, 500 },
        .phase_lines = {
            "The Auditor queries several acquisitions.",
            "\"These receipts are provisional at best.\"",
            "\"I withdraw my objection.\""
        },
        .reward_item = ITEM_FRAGMENT_II
    },
    [BOSS_VEX] = {
        .type = { "Vex", 600, 34, 22, 20, 0, 0 },
        .phase_lines = {
            "\"You have no idea what I have become.\"",
            "\"Why does nobody remember me before the crown?\"",
            "\"I never broke it. Did I? Did I.\""
        },
        .reward_item = ITEM_FRAGMENT_III
    }
};

const BossType *battle_boss(BossId id)
{
    if (id < 0 || id >= BOSS_COUNT) {
        return NULL;
    }

    return &boss_table[id];
}

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

/* The same thing, but formatted -- so callers stop declaring a scratch
   buffer every time they want to say something with a number in it.

   The format attribute asks gcc to type-check the arguments against the
   format string exactly as it does for printf. Without it a variadic
   function is a hole in the type system: nothing else checks that a %d
   was given an int. */
__attribute__((format(printf, 2, 3)))
static void log_linef(Battle *battle, const char *fmt, ...)
{
    char line[BATTLE_LOG_LEN];

    va_list args;
    va_start(args, fmt);
    /* vsnprintf is the va_list-taking twin of snprintf. There is a v*
       version of every printf variant precisely so wrappers like this
       one can forward the arguments they were handed. */
    vsnprintf(line, sizeof line, fmt, args);
    va_end(args);

    log_line(battle, line);
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

    /* memset zeroes boss_id, and zero is a valid BossId -- so say "not a
       boss" explicitly, before any early return can leave it saying
       "Bogwright". */
    battle->boss_id = -1;

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

    assert(count >= 1 && count <= BATTLE_MAX_ENEMIES);

    battle->enemy_count = count;
    battle->target = 0;
    battle->outcome = BATTLE_ONGOING;

    if (count == 1) {
        log_linef(battle, "A %s blocks the way.",
                  battle->enemies[0].type->name);
    } else {
        log_linef(battle, "%d enemies block the way.", count);
    }
}

void battle_begin_boss(Battle *battle, BossId id)
{
    memset(battle, 0, sizeof *battle);
    battle->boss_id = -1;

    const BossType *boss = battle_boss(id);
    if (boss == NULL) {
        battle->outcome = BATTLE_WON;
        return;
    }

    battle->enemies[0].type = &boss->type;
    battle->enemies[0].hp = boss->type.max_hp;
    battle->enemies[0].alive = 1;
    battle->enemy_count = 1;
    battle->target = 0;
    battle->outcome = BATTLE_ONGOING;
    battle->boss_id = (int)id;
    battle->boss_phase = 0;

    log_linef(battle, "%s.", boss->type.name);
    log_line(battle, boss->phase_lines[0]);
}

int battle_boss_phase(const Battle *battle)
{
    if (battle->boss_id < 0 || battle->enemy_count < 1) {
        return 0;
    }

    const Enemy *enemy = &battle->enemies[0];
    int max_hp = enemy->type->max_hp;
    if (max_hp <= 0) {
        return 0;
    }

    /* Multiply rather than divide: (hp * 3) / max_hp keeps the whole
       calculation in integers without losing the fraction to rounding. */
    int thirds = (enemy->hp * 3) / max_hp;

    if (thirds >= 2) {
        return 0;
    }
    if (thirds >= 1) {
        return 1;
    }
    return 2;
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

    /* Every read of enemies[] downstream depends on this. An assert
       states the assumption where it is made, so a future change that
       breaks it stops here rather than corrupting memory later. */
    assert(battle->target >= 0 && battle->target < battle->enemy_count);

    Enemy *enemy = &battle->enemies[battle->target];
    if (!enemy->alive) {
        return;
    }

    if (!combat_roll_hit(rng, entity_agility(player), enemy->type->agility)) {
        log_linef(battle, "You swing at the %s and miss.",
                  enemy->type->name);
        return;
    }

    int critical = 0;
    int damage = combat_roll_damage(rng, entity_attack(player), 0,
                                    enemy->type->defense, &critical);

    enemy->hp -= damage;

    log_linef(battle, "%s the %s for %d.",
             critical ? "CRITICAL on" : "You hit", enemy->type->name, damage);

    if (enemy->hp <= 0) {
        enemy->hp = 0;
        enemy->alive = 0;

        battle->xp_reward += enemy->type->xp;
        battle->gold_reward += enemy->type->gold;

        log_linef(battle, "The %s falls.", enemy->type->name);
    }
}

static void enemy_attacks(Battle *battle, Player *player, Rng *rng,
                          const Enemy *enemy, int enemy_damage_pct)
{
    if (enemy->status & STATUS_ASLEEP) {
        log_linef(battle, "The %s sleeps on.", enemy->type->name);
        return;
    }

    if (!combat_roll_hit(rng, enemy->type->agility, entity_agility(player))) {
        log_linef(battle, "The %s misses you.", enemy->type->name);
        return;
    }

    int attack = enemy->type->attack;

    /* A cornered boss fights harder: a fifth more attack in the last
       phase. Derived from HP like everything else about phases, so it
       cannot drift out of step with what the player can see. */
    if (battle->boss_id >= 0 && battle_boss_phase(battle) == 2) {
        attack += attack / 5;
    }

    /* Dull shaves a quarter off the enemy's attack while it lasts. */
    if (enemy->status & STATUS_DULLED) {
        attack -= attack / 4;
    }

    int critical = 0;
    int damage = combat_roll_damage(rng, attack, 0,
                                    entity_defense(player), &critical);

    /* Apply the difficulty multiplier, never letting it round a real
       hit down to nothing. */
    damage = (damage * enemy_damage_pct) / 100;
    if (damage < 1) {
        damage = 1;
    }

    /* Ward soaks the blow entirely, once, then breaks. */
    if (player->status & STATUS_WARDED) {
        player->status &= ~STATUS_WARDED;
        player->status_turns = 0;
        log_linef(battle, "The ward absorbs the %s's blow.",
                 enemy->type->name);
        return;
    }

    player->hp -= damage;
    if (player->hp < 0) {
        player->hp = 0;
    }

    log_linef(battle, "The %s hits you for %d.%s",
             enemy->type->name, damage, critical ? " Critical." : "");
}

/* If a boss has crossed into a new phase, say so -- once. The phase is
   derived from HP, so this is the only place that needs to remember
   anything, and it remembers the smallest possible thing. */
static void announce_boss_phase(Battle *battle)
{
    if (battle->boss_id < 0) {
        return;
    }

    int phase = battle_boss_phase(battle);
    if (phase <= battle->boss_phase) {
        return;
    }

    battle->boss_phase = phase;

    const BossType *boss = battle_boss((BossId)battle->boss_id);
    if (boss != NULL) {
        log_line(battle, boss->phase_lines[phase]);
    }
}

void battle_enemy_turn(Battle *battle, Player *player, Rng *rng,
                       int enemy_damage_pct)
{
    if (battle->outcome != BATTLE_ONGOING) {
        return;
    }

    announce_boss_phase(battle);

    if (battle_living_enemies(battle) == 0) {
        battle->outcome = BATTLE_WON;
        return;
    }

    for (int i = 0; i < battle->enemy_count; i++) {
        const Enemy *enemy = &battle->enemies[i];
        if (enemy->alive) {
            enemy_attacks(battle, player, rng, enemy, enemy_damage_pct);

            if (player->hp <= 0) {
                battle->outcome = BATTLE_LOST;
                return;
            }
        }
    }

    magic_tick_status(&player->status, &player->status_turns);
    for (int i = 0; i < battle->enemy_count; i++) {
        if (battle->enemies[i].alive) {
            magic_tick_status(&battle->enemies[i].status,
                              &battle->enemies[i].status_turns);
        }
    }

    ensure_valid_target(battle);
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

        int chance = combat_flee_chance(entity_agility(player), fastest);
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
        if (enemy->alive && enemy->type->agility > entity_agility(player)) {
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

        /* The hero's blow may have pushed a boss into its next phase. */
        announce_boss_phase(battle);
    }

    /* Then everyone the hero outpaced (ties included -- hero wins ties). */
    for (int i = 0; i < battle->enemy_count; i++) {
        const Enemy *enemy = &battle->enemies[i];
        if (enemy->alive && enemy->type->agility <= entity_agility(player)) {
            enemy_attacks(battle, player, rng, enemy, enemy_damage_pct);

            if (player->hp <= 0) {
                battle->outcome = BATTLE_LOST;
                return;
            }
        }
    }

    /* Statuses last a number of rounds, not a number of actions. */
    magic_tick_status(&player->status, &player->status_turns);
    for (int i = 0; i < battle->enemy_count; i++) {
        if (battle->enemies[i].alive) {
            magic_tick_status(&battle->enemies[i].status,
                              &battle->enemies[i].status_turns);
        }
    }

    ensure_valid_target(battle);
}
