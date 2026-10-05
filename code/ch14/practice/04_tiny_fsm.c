#include <stdio.h>
typedef enum { ST_MENU, ST_WALK, ST_QUIT, ST_COUNT } State;
typedef struct Game Game;
typedef void (*Handle)(Game *g, int key);
struct Game { State state; int steps; };
static void menu_h(Game *g, int key);
static void walk_h(Game *g, int key);
static void quit_h(Game *g, int key);
static const Handle table[ST_COUNT] = { menu_h, walk_h, quit_h };
static void menu_h(Game *g, int key)
{
    if (key == 'w') g->state = ST_WALK;
    if (key == 'q') g->state = ST_QUIT;
}
/* TODO: implement walk transitions (m->menu, q->quit, else steps++) */
static void walk_h(Game *g, int key);
static void quit_h(Game *g, int key){ (void)g;(void)key; }
static int fails;
static void check(int c, const char *m){ if(!c){ fprintf(stderr,"FAIL: %s\n",m); fails++; } }
int main(void)
{
    Game g = { .state = ST_MENU, .steps = 0 };
    table[g.state](&g, 'w');
    check(g.state == ST_WALK, "enter walk");
    table[g.state](&g, 'x');
    table[g.state](&g, 'x');
    check(g.steps == 2, "steps");
    table[g.state](&g, 'q');
    check(g.state == ST_QUIT, "quit");
    if (fails) { fprintf(stderr, "%d failed\n", fails); return 1; }
    puts("04_tiny_fsm: all checks passed");
    return 0;
}
