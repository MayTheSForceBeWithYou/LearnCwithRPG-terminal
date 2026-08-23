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
    AREA_COUNT
} AreaId;

#define WORLD_MAX_NPCS_PER_AREA 4
#define WORLD_MAX_WARPS_PER_AREA 2

typedef struct {
    int x;
    int y;
    AreaId destination;
    int destination_x;
    int destination_y;
} Warp;

typedef struct {
    const char *name;
    const char *map_path;
    Npc npcs[WORLD_MAX_NPCS_PER_AREA];
    int npc_count;
    Warp warps[WORLD_MAX_WARPS_PER_AREA];
    int warp_count;
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

#endif
