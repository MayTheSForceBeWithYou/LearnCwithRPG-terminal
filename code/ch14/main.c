#include <stdio.h>
#include "game.h"
#include "map.h"
#include "render.h"
#include "input.h"
#include "duel.h"

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*       SOME ASSEMBLY REQUIRED         *\n");
    printf("*       A Quest in Four Pieces         *\n");
    printf("****************************************\n");

    Game *game = game_create();
    if (game == NULL) {
        fprintf(stderr, "Could not start the game. See the message above.\n");
        return 1;
    }

    entity_print_sheet(&game->player);

    printf("\n");
    printf("The Guild's commission is in your pocket. Press Enter...\n");
    getchar();

    if (!render_init()) {
        printf("Could not start the display. Is $TERM set correctly?\n");
        game_destroy(game);
        return 1;
    }

    while (game->running) {
        const Map *map = world_current_map(game->world);

        camera_center_on(&game->camera, game->player.x, game->player.y,
                         map->width, map->height);

        game_draw(game);
        game_handle(game, input_poll());
    }

    render_shutdown();

    duel_run();

    game_destroy(game);

    return 0;
}
