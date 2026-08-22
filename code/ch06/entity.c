#include <stdio.h>
#include "entity.h"

static char rank_to_char(Rank rank)
{
    switch (rank) {
        case RANK_E: return 'E';
        case RANK_D: return 'D';
        case RANK_C: return 'C';
        case RANK_B: return 'B';
        case RANK_A: return 'A';
        case RANK_S: return 'S';
    }
    return '?';
}

Player entity_create_player(void)
{
    Player p;

    printf("What is your name, hero? ");
    fgets(p.name, sizeof p.name, stdin);

    p.x = 1;
    p.y = 1;
    p.hp = 20;
    p.mp = 8;
    p.attack = 6;
    p.defense = 4;
    p.rank = RANK_E;

    return p;
}

void entity_print_sheet(Player p)
{
    float power_rating = (float)(p.attack + p.defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", p.name);
    printf("----------------------------------------\n");
    printf("HP:     %d\n", p.hp);
    printf("MP:     %d\n", p.mp);
    printf("ATK:    %d\n", p.attack);
    printf("DEF:    %d\n", p.defense);
    printf("Rank:   %c\n", rank_to_char(p.rank));
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}

void entity_move(Player p, int dx, int dy)
{
    /* This looks right. It compiles clean. It does not work -- p is a
       copy, so changing it here changes nothing the caller can see.
       Chapter 7 explains exactly why, and fixes it for good. */
    p.x += dx;
    p.y += dy;
}
