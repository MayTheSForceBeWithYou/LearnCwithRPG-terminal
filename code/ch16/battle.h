#ifndef BATTLE_H
#define BATTLE_H

#include "entity.h"
#include "rng.h"

#define BATTLE_MAX_ENEMIES 3
#define BATTLE_LOG_LINES   4
#define BATTLE_LOG_LEN     48

typedef struct {
    const char *name;
    int max_hp;
    int attack;
    int defense;
    int agility;
    int xp;
    int gold;
} EnemyType;

typedef struct {
    const EnemyType *type;
    int hp;
    int alive;
} Enemy;

typedef enum {
    BATTLE_ONGOING,
    BATTLE_WON,
    BATTLE_LOST,
    BATTLE_FLED
} BattleOutcome;

typedef struct {
    Enemy enemies[BATTLE_MAX_ENEMIES];
    int enemy_count;
    int target;                 /* index of the currently selected enemy */
    BattleOutcome outcome;
    int xp_reward;
    int gold_reward;
    char log[BATTLE_LOG_LINES][BATTLE_LOG_LEN];
    int log_count;
} Battle;

/* Fills a battle with enemies drawn from the given area's roster.

   The group size is capped by hero_level as well as by the roster: a
   solo hero attacks once per round while every enemy attacks, so facing
   three at level 1 is not a challenge, it is arithmetic. See the chapter
   for the measurements behind this. */
void battle_begin(Battle *battle, Rng *rng, int area_index, int hero_level);

/* How many enemies a hero of this level may face at once. */
int battle_max_group_for_level(int hero_level);

/* Runs one full round: the hero's chosen action, then every surviving
   enemy, in AGI order. Sets battle->outcome. */
void battle_round(Battle *battle, Player *player, Rng *rng, int fleeing);

int battle_living_enemies(const Battle *battle);

/* Moves the target cursor to the next living enemy. */
void battle_cycle_target(Battle *battle, int direction);

#endif
