#include <stdio.h>
#include <stdlib.h>
#include "world.h"
#include "inventory.h"
#include "battle.h"
#include "paths.h"

/* The world, as data. Adding a village means adding a row here and an
   AreaId above -- not writing new code. Index order must match AreaId. */
static const Area area_table[AREA_COUNT] = {
    [AREA_GRUBBIN_VALE] = {
        .name = "Grubbin Vale",
        .map_path = "maps/grubbin_vale.map",
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
              .destination_x = 1, .destination_y = 5,
              .requires_item = -1 },
            { .x = 20, .y = 19, .destination = AREA_CASTLE_HOLLIS,
              .destination_x = 18, .destination_y = 1,
              .requires_item = -1 }
        },
        .warp_count = 2,
        .boss = { .boss_id = -1 }
    },

    [AREA_WETWOOD_BARROW] = {
        .name = "Wetwood Barrow",
        .map_path = "maps/wetwood_barrow.map",
        .npcs = {
            { .x = 12, .y = 5, .speech =
                "Something got into the turnips. Not a fox. Foxes don't "
                "do that to a fence." }
        },
        .npc_count = 1,
        .warps = {
            { .x = 0, .y = 5, .destination = AREA_GRUBBIN_VALE,
              .destination_x = 38, .destination_y = 4,
              .requires_item = -1 }
        },
        .warp_count = 1,
        /* Fragment I. The Bogwright has been guarding this since before
           anyone now living was told to stop. */
        .boss = { .x = 20, .y = 9, .boss_id = BOSS_BOGWRIGHT }
    },

    [AREA_CASTLE_HOLLIS] = {
        .name = "Castle Hollis",
        .map_path = "maps/castle_hollis.map",
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
              .destination_x = 20, .destination_y = 18,
              .requires_item = -1 },

            /* The story's order, expressed as two locked doors rather
               than a quest-flag system. */
            { .x = 7, .y = 15, .destination = AREA_MUDWICK,
              .destination_x = 21, .destination_y = 11,
              .requires_item = ITEM_FRAGMENT_I,
              .locked_speech =
                "The south road is closed to unaccredited persons. The "
                "Guild suggests returning with something to show." },

            { .x = 28, .y = 15, .destination = AREA_THE_CROWNLESS_COURT,
              .destination_x = 10, .destination_y = 11,
              .requires_item = ITEM_FRAGMENT_II,
              .locked_speech =
                "The door to the old court is sealed. Whatever opens it, "
                "you are not carrying enough of it yet." }
        },
        .warp_count = 3,
        .boss = { .boss_id = -1 }
    },

    [AREA_MUDWICK] = {
        .name = "Mudwick",
        .map_path = "maps/mudwick.map",
        .npcs = {
            { .x = 12, .y = 6, .shopkeeper = 1, .speech =
                "Room's ninety. It's not worth ninety. Nothing here is "
                "worth what it costs any more." },
            { .x = 8, .y = 11, .speech =
                "Town's sinking. Has been for years. We've stopped "
                "writing it in the minutes -- everyone knows." },
            { .x = 20, .y = 5, .speech =
                "A man came through, asking the same questions as you. "
                "Polite. Didn't blink enough." }
        },
        .npc_count = 3,
        .warps = {
            { .x = 21, .y = 12, .destination = AREA_CASTLE_HOLLIS,
              .destination_x = 7, .destination_y = 14,
              .requires_item = -1 },
            { .x = 5, .y = 0, .destination = AREA_THE_SUMP,
              .destination_x = 26, .destination_y = 11,
              .requires_item = -1 }
        },
        .warp_count = 2,
        .boss = { .boss_id = -1 }
    },

    [AREA_THE_SUMP] = {
        .name = "The Sump",
        .map_path = "maps/the_sump.map",
        .npcs = {
            { .x = 24, .y = 10, .speech =
                "Surveyor's mark, that. Dated eleven years ago. Nobody's "
                "been down since to correct it." }
        },
        .npc_count = 1,
        .warps = {
            { .x = 27, .y = 11, .destination = AREA_MUDWICK,
              .destination_x = 5, .destination_y = 1,
              .requires_item = -1 }
        },
        .warp_count = 1,
        /* Fragment II. */
        .boss = { .x = 14, .y = 4, .boss_id = BOSS_SUMP_AUDITOR }
    },

    [AREA_THE_CROWNLESS_COURT] = {
        .name = "The Crownless Court",
        .map_path = "maps/the_crownless_court.map",
        .npcs = {
            { .x = 4, .y = 10, .speech =
                "Perrin Culch is sitting on the step with his ledger "
                "closed. \"I filed it,\" he says. \"Eleven years ago. I "
                "tripped on the rug and I filed a report about a "
                "sorcerer, and then everyone believed me, and then he "
                "was there. I have wanted to say that for a long time.\"" }
        },
        .npc_count = 1,
        .warps = {
            { .x = 1, .y = 11, .destination = AREA_CASTLE_HOLLIS,
              .destination_x = 28, .destination_y = 14,
              .requires_item = -1 }
        },
        .warp_count = 1,
        .boss = { .x = 10, .y = 1, .boss_id = BOSS_VEX }
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
        char full[512];
        paths_data(full, sizeof full, world->areas[i].map_path);

        world->maps[i] = map_load(full);
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

const BossTrigger *world_boss_at(const World *world, int x, int y)
{
    const Area *area = world_current_area(world);

    if (area->boss.boss_id < 0) {
        return NULL;
    }

    if (area->boss.x == x && area->boss.y == y) {
        return &area->boss;
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
