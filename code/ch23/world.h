#ifndef WORLD_H
#define WORLD_H

#include "map.h"
#include "entity.h"

/* One entry per place the player can be. The order here must match the
   order of the table in world.c -- that is what makes an AreaId usable
   as an array index. */
typedef enum {
    AREA_GRUBBIN_VALE,
    AREA_WETWOOD_BARROW,
    AREA_CASTLE_HOLLIS,
    AREA_MUDWICK,
    AREA_THE_SUMP,
    AREA_THE_CROWNLESS_COURT,
    AREA_COUNT
} AreaId;

#define WORLD_MAX_NPCS_PER_AREA 4
#define WORLD_MAX_WARPS_PER_AREA 3

typedef struct {
    int x;
    int y;
    AreaId destination;
    int destination_x;
    int destination_y;

    /* Progress gating. A warp with requires_item set to an item id only
       opens once that key item is carried; -1 means always open. This is
       how the story keeps its order without a quest-flag system: the
       fragments you are already carrying *are* the progress record. */
    int requires_item;
    const char *locked_speech;
} Warp;

/* A tile that starts a boss fight when stepped on. A boss whose reward
   item is already in the inventory has been beaten, so the trigger goes
   quiet -- again, no separate flag to save. */
typedef struct {
    int x;
    int y;
    int boss_id;          /* index into the boss table in battle.c */
} BossTrigger;

typedef struct {
    const char *name;
    const char *map_path;
    Npc npcs[WORLD_MAX_NPCS_PER_AREA];
    int npc_count;
    Warp warps[WORLD_MAX_WARPS_PER_AREA];
    int warp_count;
    BossTrigger boss;     /* boss_id < 0 means the area has no boss */
} Area;

typedef struct {
    Area areas[AREA_COUNT];
    Map *maps[AREA_COUNT];
    AreaId current;
} World;

/* Paired lifetime: every world_create must be matched by exactly one
   world_destroy. Loads every area's map up front; returns NULL (having
   freed anything it had already loaded) if any of them fail. */
World *world_create(void);
void world_destroy(World *world);

const Area *world_current_area(const World *world);
Map *world_current_map(const World *world);

/* Returns the NPC standing at (x, y) in the current area, or NULL. */
const Npc *world_npc_at(const World *world, int x, int y);

/* Returns the warp at (x, y) in the current area, or NULL. */
const Warp *world_warp_at(const World *world, int x, int y);

/* The boss trigger on this tile, or NULL. Does not consider whether the
   boss has already been beaten -- the caller checks the reward item. */
const BossTrigger *world_boss_at(const World *world, int x, int y);

#endif
