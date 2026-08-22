#include <stdio.h>
#include "entity.h"
#include "party.h"

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
    p.level = 1;
    p.xp = 0;
    p.gold = 40;      /* the Guild's advance, such as it is */
    p.rank = RANK_E;

    /* Stats come from the level table rather than being written here,
       so there is exactly one place they are defined. */
    party_apply_level(&p);
    p.hp = p.max_hp;
    p.mp = p.max_mp;

    return p;
}

void entity_print_sheet(const Player *p)
{
    float power_rating = (float)(p->attack + p->defense) / 2.0f;

    printf("\n");
    printf("Welcome, %s", p->name);
    printf("----------------------------------------\n");
    printf("Level:  %d\n", p->level);
    printf("HP:     %d/%d\n", p->hp, p->max_hp);
    printf("MP:     %d/%d\n", p->mp, p->max_mp);
    printf("Gold:   %d\n", p->gold);
    printf("ATK:    %d\n", p->attack);
    printf("DEF:    %d\n", p->defense);
    printf("Rank:   %c\n", rank_to_char(p->rank));
    printf("Power:  %.1f\n", power_rating);
    printf("----------------------------------------\n");
}

void entity_move(Player *p, int dx, int dy)
{
    p->x += dx;
    p->y += dy;
}
