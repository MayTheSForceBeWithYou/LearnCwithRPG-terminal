#include <stdio.h>
#include "entity.h"
#include "map.h"
#include "world.h"
#include "camera.h"
#include "render.h"
#include "input.h"
#include "dialog.h"
#include "duel.h"

#define TEXTBOX_WIDTH (VIEW_WIDTH - 2)

/* The modes the game can be in. For now there are only two, and the
   switch below handles both inline. Chapter 14 rebuilds this. */
typedef enum {
    MODE_EXPLORE,
    MODE_DIALOG
} GameMode;

static void draw_area(const World *world, const Player *player,
                      const Camera *cam, const Dialog *dialog,
                      GameMode mode)
{
    const Map *map = world_current_map(world);
    const Area *area = world_current_area(world);

    render_clear();

    for (int screen_y = 0; screen_y < VIEW_HEIGHT; screen_y++) {
        for (int screen_x = 0; screen_x < VIEW_WIDTH; screen_x++) {
            int world_x = cam->x + screen_x;
            int world_y = cam->y + screen_y;

            char tile = map_tile_at(map, world_x, world_y);
            TileType t = TILE_WALL;
            if (tile == '.') {
                t = TILE_FLOOR;
            } else if (tile == '+') {
                t = TILE_DOOR;
            }

            render_draw_tile(screen_x, screen_y, t);
        }
    }

    /* Only draw NPCs that actually fall inside the viewport. Without
       this check, an NPC standing off-screen is drawn at a screen
       coordinate outside the map area -- on top of the status line. */
    for (int i = 0; i < area->npc_count; i++) {
        int sx = camera_world_to_screen_x(cam, area->npcs[i].x);
        int sy = camera_world_to_screen_y(cam, area->npcs[i].y);

        if (sx >= 0 && sx < VIEW_WIDTH && sy >= 0 && sy < VIEW_HEIGHT) {
            render_draw_tile(sx, sy, TILE_NPC);
        }
    }

    render_draw_tile(camera_world_to_screen_x(cam, player->x),
                     camera_world_to_screen_y(cam, player->y),
                     TILE_PLAYER);

    render_draw_text(0, VIEW_HEIGHT, area->name);

    if (mode == MODE_DIALOG) {
        for (int i = 0; i < dialog->line_count; i++) {
            render_draw_text(1, VIEW_HEIGHT + 1 + i, dialog->lines[i]);
        }
    } else {
        render_draw_text(0, VIEW_HEIGHT + 1,
                         "w/a/s/d move, q quit");
    }

    render_present();
}

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*       SOME ASSEMBLY REQUIRED         *\n");
    printf("*       A Quest in Four Pieces         *\n");
    printf("****************************************\n");

    World *world = world_create();
    if (world == NULL) {
        fprintf(stderr, "Could not build the world. See the message above.\n");
        return 1;
    }

    Player player = entity_create_player();
    entity_print_sheet(&player);

    player.x = 2;
    player.y = 2;

    Dialog dialog;
    dialog.line_count = 0;

    printf("\n");
    printf("The Guild's commission is in your pocket. Press Enter...\n");
    getchar();

    if (!render_init()) {
        printf("Could not start the display. Is $TERM set correctly?\n");
        world_destroy(world);
        return 1;
    }

    Camera camera;
    GameMode mode = MODE_EXPLORE;
    InputEvent event = INPUT_NONE;

    while (event != INPUT_QUIT) {
        const Map *map = world_current_map(world);

        camera_center_on(&camera, player.x, player.y,
                         map->width, map->height);
        draw_area(world, &player, &camera, &dialog, mode);

        event = input_poll();

        /* One switch per mode, each doing its own input handling. This
           works, and it is already awkward -- note how much of the
           movement logic sits nested three levels deep, and how adding a
           MENU mode would mean another block just like it. Chapter 14
           fixes this properly. */
        switch (mode) {
            case MODE_EXPLORE: {
                int dx = 0;
                int dy = 0;

                switch (event) {
                    case INPUT_UP:    dy = -1; break;
                    case INPUT_DOWN:  dy = 1;  break;
                    case INPUT_LEFT:  dx = -1; break;
                    case INPUT_RIGHT: dx = 1;  break;
                    case INPUT_QUIT:  break;
                    case INPUT_NONE:  break;
                }

                if (dx != 0 || dy != 0) {
                    int target_x = player.x + dx;
                    int target_y = player.y + dy;

                    const Npc *npc = world_npc_at(world, target_x, target_y);
                    if (npc != NULL) {
                        dialog_wrap(&dialog, npc->speech, TEXTBOX_WIDTH);
                        mode = MODE_DIALOG;
                    } else if (map_is_walkable(map, target_x, target_y)) {
                        entity_move(&player, dx, dy);

                        const Warp *warp =
                            world_warp_at(world, player.x, player.y);
                        if (warp != NULL) {
                            world->current = warp->destination;
                            player.x = warp->destination_x;
                            player.y = warp->destination_y;
                        }
                    }
                }
                break;
            }

            case MODE_DIALOG:
                /* Any key dismisses the text box. */
                if (event != INPUT_NONE) {
                    mode = MODE_EXPLORE;
                }
                break;
        }
    }

    render_shutdown();

    duel_run();

    world_destroy(world);

    return 0;
}
