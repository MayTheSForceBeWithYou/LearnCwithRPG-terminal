#include <stdio.h>
#include "entity.h"
#include "map.h"
#include "render.h"
#include "input.h"
#include "duel.h"

static void draw_world(const Player *player)
{
    render_clear();

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            TileType t = map_is_walkable(x, y) ? TILE_FLOOR : TILE_WALL;
            render_draw_tile(x, y, t);
        }
    }

    render_draw_tile(player->x, player->y, TILE_PLAYER);
    render_draw_text(0, MAP_HEIGHT + 1, "w/a/s/d to move, q to quit wandering");

    render_present();
}

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*          UNTITLED JRPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");

    Player player = entity_create_player();
    entity_print_sheet(&player);

    printf("\n");
    printf("The village gate creaks open. Press Enter to step through...\n");
    getchar();

    if (!render_init()) {
        printf("Could not start the display. Is $TERM set correctly?\n");
        return 1;
    }

    InputEvent event = INPUT_NONE;
    while (event != INPUT_QUIT) {
        draw_world(&player);

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

            if (map_is_walkable(target_x, target_y)) {
                entity_move(&player, dx, dy);
            }
        }
    }

    render_shutdown();

    duel_run();

    return 0;
}
