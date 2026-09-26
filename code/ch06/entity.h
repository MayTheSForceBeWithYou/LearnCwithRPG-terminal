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
    int x;              /* coordinates can be negative (offsets, wrapping) */
    int y;
    int hp;             /* could use unsigned; kept as int for arithmetic */
    int mp;             /* (damage deltas, library interop, avoiding casts) */
    int attack;         /* see Chapter 2's "note on type choices" */
    int defense;
    Rank rank;
} Player;

Player entity_create_player(void);
void entity_print_sheet(Player p);
void entity_move(Player p, int dx, int dy);

#endif
