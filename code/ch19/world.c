#include <stdio.h>
#include <stdlib.h>
#include "world.h"

/* The world, as data. Adding a village means adding a row here and an
   AreaId above -- not writing new code. Index order must match AreaId. */
static const Area area_table[AREA_COUNT] = {
    [AREA_GRUBBIN_VALE] = {
        .name = "Grubbin Vale",
        .map_path = "assets/maps/grubbin_vale.map",
        .npcs = {
            { .x = 8, .y = 6, .shopkeeper = 1, .speech =
                "Bed's six gold. Sleep fixes most things, in my "
                "experience. Not everything. Most." },
            { .x = 15, .y = 11, .speech =
                "Four pieces. Scattered north, south, and -- the other "
                "two directions. I had it written down." },
            { .x = 26, .y = 16, .speech =
                "The Guild is prepared to offer forty gold up front and "
                "a completion bonus that I'd rather not describe as "
                "generous. Eleven others declined." }
        },
        .npc_count = 3,
        .warps = {
            { .x = 39, .y = 4, .destination = AREA_WETWOOD_BARROW,
              .destination_x = 1, .destination_y = 5 },
            { .x = 20, .y = 19, .destination = AREA_CASTLE_HOLLIS,
              .destination_x = 18, .destination_y = 1 }
        },
        .warp_count = 2
    },

    [AREA_WETWOOD_BARROW] = {
        .name = "Wetwood Barrow",
        .map_path = "assets/maps/wetwood_barrow.map",
        .npcs = {
            { .x = 12, .y = 5, .speech =
                "Something got into the turnips. Not a fox. Foxes don't "
                "do that to a fence." }
        },
        .npc_count = 1,
        .warps = {
            { .x = 0, .y = 5, .destination = AREA_GRUBBIN_VALE,
              .destination_x = 38, .destination_y = 4 }
        },
        .warp_count = 1
    },

    [AREA_CASTLE_HOLLIS] = {
        .name = "Castle Hollis",
        .map_path = "assets/maps/castle_hollis.map",
        .npcs = {
            { .x = 10, .y = 8, .speech =
                "Sorry -- is this about the vault inventory? It's not "
                "ready. It's been eleven years and it's not ready." },
            { .x = 28, .y = 14, .shopkeeper = 1, .speech =
                "Everything's twice what it was. That's not me gouging. "
                "That's the roads." }
        },
        .npc_count = 2,
        .warps = {
            { .x = 18, .y = 0, .destination = AREA_GRUBBIN_VALE,
              .destination_x = 20, .destination_y = 18 }
        },
        .warp_count = 1
    }
};

World *world_create(void)
{
    World *world = malloc(sizeof *world);
    if (world == NULL) {
        fprintf(stderr, "world_create: out of memory\n");
        return NULL;
    }

    for (int i = 0; i < AREA_COUNT; i++) {
        world->areas[i] = area_table[i];
        world->maps[i] = NULL;
    }

    for (int i = 0; i < AREA_COUNT; i++) {
        world->maps[i] = map_load(world->areas[i].map_path);
        if (world->maps[i] == NULL) {
            fprintf(stderr, "world_create: could not load area '%s'\n",
                    world->areas[i].name);
            /* Release the maps loaded before this one, then the World. */
            world_destroy(world);
            return NULL;
        }
    }

    world->current = AREA_GRUBBIN_VALE;
    return world;
}

void world_destroy(World *world)
{
    if (world == NULL) {
        return;
    }

    for (int i = 0; i < AREA_COUNT; i++) {
        map_destroy(world->maps[i]);
    }

    free(world);
}

const Area *world_current_area(const World *world)
{
    return &world->areas[world->current];
}

Map *world_current_map(const World *world)
{
    return world->maps[world->current];
}

const Npc *world_npc_at(const World *world, int x, int y)
{
    const Area *area = world_current_area(world);

    for (int i = 0; i < area->npc_count; i++) {
        if (area->npcs[i].x == x && area->npcs[i].y == y) {
            return &area->npcs[i];
        }
    }

    return NULL;
}

const Warp *world_warp_at(const World *world, int x, int y)
{
    const Area *area = world_current_area(world);

    for (int i = 0; i < area->warp_count; i++) {
        if (area->warps[i].x == x && area->warps[i].y == y) {
            return &area->warps[i];
        }
    }

    return NULL;
}
