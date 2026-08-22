#include <stdio.h>
#include "character.h"

void character_intro(void)
{
    char name[64];
    printf("What is your name, hero? ");
    fgets(name, sizeof name, stdin);

    int hp = 20;
    int mp = 8;
    int attack = 6;
    int defense = 4;
    char rank = 'E';
    float power_rating = (float)(attack + defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", name);
    printf("----------------------------------------\n");
    printf("HP:     %d\n", hp);
    printf("MP:     %d\n", mp);
    printf("ATK:    %d\n", attack);
    printf("DEF:    %d\n", defense);
    printf("Rank:   %c\n", rank);
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}
