#ifndef ENTITY_H
#define ENTITY_H

typedef enum {
    RANK_E,
    RANK_D,
    RANK_C,
    RANK_B,
    RANK_A,
    RANK_S
} Rank;

typedef struct {
    char name[64];
    int x;
    int y;
    int hp;
    int mp;
    int attack;
    int defense;
    Rank rank;
} Player;

Player entity_create_player(void);
void entity_print_sheet(const Player *p);
void entity_move(Player *p, int dx, int dy);

#endif
