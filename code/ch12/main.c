#include <stdio.h>
#include "entity.h"
#include "map.h"
#include "camera.h"
#include "render.h"
#include "input.h"
#include "dialog.h"
#include "duel.h"

#define TEXTBOX_WIDTH (VIEW_WIDTH - 2)

static void draw_textbox(const Dialog *dialog)
{
    int top = VIEW_HEIGHT + 1;

    for (int i = 0; i < dialog->line_count; i++) {
        render_draw_text(1, top + i, dialog->lines[i]);
    }
}

static void draw_world(const Map *map, const Player *player,
                       const Npc *npc, const Camera *cam,
                       const Dialog *dialog, int talking)
{
    render_clear();

    for (int screen_y = 0; screen_y < VIEW_HEIGHT; screen_y++) {
        for (int screen_x = 0; screen_x < VIEW_WIDTH; screen_x++) {
            int world_x = cam->x + screen_x;
            int world_y = cam->y + screen_y;

            TileType t = map_is_walkable(map, world_x, world_y)
                       ? TILE_FLOOR
                       : TILE_WALL;

            render_draw_tile(screen_x, screen_y, t);
        }
    }

    render_draw_tile(camera_world_to_screen_x(cam, npc->x),
                     camera_world_to_screen_y(cam, npc->y),
                     TILE_NPC);

    render_draw_tile(camera_world_to_screen_x(cam, player->x),
                     camera_world_to_screen_y(cam, player->y),
                     TILE_PLAYER);

    if (talking) {
        draw_textbox(dialog);
    } else {
        render_draw_text(0, VIEW_HEIGHT + 1,
                         "w/a/s/d to move, q to quit wandering");
    }

    render_present();
}

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*          UNTITLED JRPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");

    Map *map = map_load("assets/maps/overworld.map");
    if (map == NULL) {
        fprintf(stderr, "Could not load the map. See the message above.\n");
        return 1;
    }

    Player player = entity_create_player();
    entity_print_sheet(&player);

    Npc gatekeeper = {
        .x = 4,
        .y = 1,
        .speech = "Careful out there. The roads have not been "
                  "safe for a long while now."
    };

    Dialog dialog;
    dialog_wrap(&dialog, gatekeeper.speech, TEXTBOX_WIDTH);

    printf("\n");
    printf("The village gate creaks open. Press Enter to step through...\n");
    getchar();

    if (!render_init()) {
        printf("Could not start the display. Is $TERM set correctly?\n");
        map_destroy(map);
        return 1;
    }

    Camera camera;
    InputEvent event = INPUT_NONE;
    int talking = 0;

    while (event != INPUT_QUIT) {
        camera_center_on(&camera, player.x, player.y,
                         map->width, map->height);
        draw_world(map, &player, &gatekeeper, &camera, &dialog, talking);

        event = input_poll();

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

            if (target_x == gatekeeper.x && target_y == gatekeeper.y) {
                /* Walking into someone starts a conversation instead. */
                talking = 1;
            } else if (map_is_walkable(map, target_x, target_y)) {
                talking = 0;
                entity_move(&player, dx, dy);
            }
        }
    }

    render_shutdown();

    duel_run();

    map_destroy(map);

    return 0;
}
