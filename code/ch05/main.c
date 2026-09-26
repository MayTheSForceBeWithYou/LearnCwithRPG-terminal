#include <stdio.h>
#include "character.h"
#include "map.h"
#include "duel.h"

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*           UNTITLED RPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
    printf("\n");

    character_intro();

    printf("\n");
    printf("The village gate creaks open.\n");
    printf("\n");
    map_print();

    duel_run();

    return 0;
}
