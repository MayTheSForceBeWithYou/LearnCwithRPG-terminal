#include <stdio.h>
#include "character.h"
#include "duel.h"

int main(void)
{
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*          UNTITLED JRPG               *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
    printf("\n");

    character_intro();
    duel_run();

    return 0;
}
