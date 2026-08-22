#include <stdio.h>
#include "entity.h"
#include "map.h"
#include "duel.h"

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*          UNTITLED JRPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");

    Player player = entity_create_player();
    entity_print_sheet(player);

    printf("\n");
    printf("The village gate creaks open.\n");
    printf("\n");
    map_print(player.x, player.y);

    printf("\n");
    printf("You try to step east...\n");
    entity_move(player, 1, 0);
    printf("Player position: (%d, %d)\n", player.x, player.y);
    printf("\n");
    map_print(player.x, player.y);

    duel_run();

    return 0;
}
