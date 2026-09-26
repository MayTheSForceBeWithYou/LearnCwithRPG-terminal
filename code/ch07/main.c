#include <stdio.h>
#include "entity.h"
#include "map.h"
#include "duel.h"

static char read_command(void)
{
    printf("Move (w/a/s/d, q to stop wandering): ");

    int typed = getchar();
    int discard = typed;

    while (discard != '\n' && discard != EOF) {
        discard = getchar();
    }

    return (char)typed;
}

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*           UNTITLED RPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");

    Player player = entity_create_player();
    entity_print_sheet(&player);

    printf("\n");
    printf("The village gate creaks open.\n");
    printf("\n");

    char command = '\0';
    while (command != 'q') {
        map_print(player.x, player.y);

        command = read_command();

        int dx = 0;
        int dy = 0;

        switch (command) {
            case 'w': dy = -1; break;
            case 's': dy = 1;  break;
            case 'a': dx = -1; break;
            case 'd': dx = 1;  break;
            case 'q': break;
            default:
                printf("Not a direction I know. Try w/a/s/d/q.\n");
                break;
        }

        if (dx != 0 || dy != 0) {
            int target_x = player.x + dx;
            int target_y = player.y + dy;

            if (map_is_walkable(target_x, target_y)) {
                entity_move(&player, dx, dy);
            } else {
                printf("You bump into a wall.\n");
            }
        }

        printf("\n");
    }

    duel_run();

    return 0;
}
