#ifndef GAME_H
#define GAME_H

#include "world.h"
#include "entity.h"
#include "camera.h"
#include "dialog.h"
#include "input.h"
#include "rng.h"
#include "battle.h"
#include "inventory.h"
#include "config.h"
#include "magic.h"
#include "shop.h"
#include "save.h"
#include <stddef.h>

/* One entry per mode the game can be in. Like AreaId, the order here
   must match the dispatch table in game.c -- that is what makes a
   GameMode usable as an array index. */
typedef enum {
    MODE_EXPLORE,
    MODE_DIALOG,
    MODE_MENU,
    MODE_BATTLE,
    MODE_ITEMS,
    MODE_SPELLS,
    MODE_GEAR,
    MODE_SHOP,
    MODE_COUNT
} GameMode;

typedef struct {
    World *world;
    Player player;
    Camera camera;
    Dialog dialog;
    GameMode mode;
    int menu_index;
    int running;
    Rng rng;
    Battle battle;
    Inventory inventory;
    Config config;
    int item_index;
    int spell_index;
    GameMode dialog_return;   /* where a text box hands control back to */
    int gear_slot;
    int shop_index;
    int steps_since_encounter;
    int encounters_enabled;   /* Checkpoint C: the player can switch these off */
} Game;

/* Paired lifetime: every game_create must be matched by exactly one
   game_destroy. Returns NULL if the world cannot be built. */
Game *game_create(void);
void game_destroy(Game *game);

/* One turn of the loop: draw the current mode, then let it handle one
   input event. */
/* Snapshots the run into a save file, and restores one. Both report a
   reason on failure. game_load leaves the game untouched if it fails. */
int game_save(Game *game, char *reason, size_t reason_size);
int game_load(Game *game, char *reason, size_t reason_size);

void game_draw(const Game *game);
void game_handle(Game *game, InputEvent event);

const char *game_mode_name(GameMode mode);

#endif
