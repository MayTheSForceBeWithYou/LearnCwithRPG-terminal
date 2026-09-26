#include <stdio.h>
#include "character.h"

void character_intro(void)
{
    char name[64];
    printf("What is your name, hero? ");
    fgets(name, sizeof name, stdin);

    unsigned hp = 20;
    unsigned mp = 8;
    unsigned attack = 6;
    unsigned defense = 4;
    char rank = 'E';
    float power_rating = (float)(attack + defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", name);
    printf("----------------------------------------\n");
    printf("HP:     %u\n", hp);
    printf("MP:     %u\n", mp);
    printf("ATK:    %u\n", attack);
    printf("DEF:    %u\n", defense);
    printf("Rank:   %c\n", rank);
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}
