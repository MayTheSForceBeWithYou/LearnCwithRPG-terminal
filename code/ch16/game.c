#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "map.h"
#include "render.h"
#include "combat_math.h"
#include "battle.h"
#include "party.h"

#define TEXTBOX_WIDTH (VIEW_WIDTH - 2)

/* Two function-pointer types: one for drawing a mode, one for handling
   an event in it. Reading a C function-pointer declaration is easier
   from the inside out: "draw_fn is a pointer to a function taking a
   const Game *, returning void." */
typedef void (*DrawFn)(const Game *game);
typedef void (*HandleFn)(Game *game, InputEvent event);

typedef struct {
    const char *name;
    DrawFn draw;
    HandleFn handle;
} ModeHandler;

static const char *const menu_items[] = {
    "Status",
    "Commission",
    "Encounters",
    "Close menu"
};

#define MENU_ITEM_COUNT ((int)(sizeof menu_items / sizeof menu_items[0]))

/* ---- shared drawing helpers ---- */

static void draw_area_tiles(const Game *game)
{
    const Map *map = world_current_map(game->world);
    const Area *area = world_current_area(game->world);
    const Camera *cam = &game->camera;

    for (int screen_y = 0; screen_y < VIEW_HEIGHT; screen_y++) {
        for (int screen_x = 0; screen_x < VIEW_WIDTH; screen_x++) {
            int world_x = cam->x + screen_x;
            int world_y = cam->y + screen_y;

            char tile = map_tile_at(map, world_x, world_y);
            TileType t = TILE_WALL;
            if (tile == '.') {
                t = TILE_FLOOR;
            } else if (tile == '+') {
                t = TILE_DOOR;
            }

            render_draw_tile(screen_x, screen_y, t);
        }
    }

    for (int i = 0; i < area->npc_count; i++) {
        int sx = camera_world_to_screen_x(cam, area->npcs[i].x);
        int sy = camera_world_to_screen_y(cam, area->npcs[i].y);

        if (sx >= 0 && sx < VIEW_WIDTH && sy >= 0 && sy < VIEW_HEIGHT) {
            render_draw_tile(sx, sy, TILE_NPC);
        }
    }

    render_draw_tile(camera_world_to_screen_x(cam, game->player.x),
                     camera_world_to_screen_y(cam, game->player.y),
                     TILE_PLAYER);

    render_draw_text(0, VIEW_HEIGHT, world_current_area(game->world)->name);
}

/* ---- EXPLORE ---- */

static void explore_draw(const Game *game)
{
    draw_area_tiles(game);
    render_draw_text(0, VIEW_HEIGHT + 1, "w/a/s/d move, m menu, q quit");
}

/* Checkpoint C: a step counter, with a minimum gap so fights never come
   back-to-back -- and an off switch the player controls. */
#define ENCOUNTER_MIN_STEPS 12
#define ENCOUNTER_PERCENT   22

static void maybe_start_encounter(Game *game)
{
    if (!game->encounters_enabled) {
        return;
    }

    game->steps_since_encounter++;

    if (game->steps_since_encounter < ENCOUNTER_MIN_STEPS) {
        return;
    }

    if (rng_range(&game->rng, 0, 99) >= ENCOUNTER_PERCENT) {
        return;
    }

    battle_begin(&game->battle, &game->rng, (int)game->world->current,
                 game->player.level);

    /* A safe area has no roster, so battle_begin hands back an
       already-won battle; don't switch modes for that. */
    if (game->battle.outcome != BATTLE_ONGOING) {
        return;
    }

    game->steps_since_encounter = 0;
    game->mode = MODE_BATTLE;
}

static void explore_handle(Game *game, InputEvent event)
{
    int dx = 0;
    int dy = 0;

    switch (event) {
        case INPUT_UP:     dy = -1; break;
        case INPUT_DOWN:   dy = 1;  break;
        case INPUT_LEFT:   dx = -1; break;
        case INPUT_RIGHT:  dx = 1;  break;
        case INPUT_MENU:   game->mode = MODE_MENU; return;
        case INPUT_CONFIRM: return;
        case INPUT_FLEE:   return;   /* nothing to flee from out here */
        case INPUT_QUIT:   game->running = 0; return;
        case INPUT_NONE:   return;
    }

    int target_x = game->player.x + dx;
    int target_y = game->player.y + dy;

    const Npc *npc = world_npc_at(game->world, target_x, target_y);
    if (npc != NULL) {
        dialog_start(&game->dialog, npc->speech, TEXTBOX_WIDTH);
        game->mode = MODE_DIALOG;
        return;
    }

    if (map_is_walkable(world_current_map(game->world), target_x, target_y)) {
        entity_move(&game->player, dx, dy);

        const Warp *warp = world_warp_at(game->world,
                                         game->player.x, game->player.y);
        if (warp != NULL) {
            game->world->current = warp->destination;
            game->player.x = warp->destination_x;
            game->player.y = warp->destination_y;
            game->steps_since_encounter = 0;
            return;
        }

        maybe_start_encounter(game);
    }
}

/* ---- DIALOG ---- */

static void dialog_draw(const Game *game)
{
    draw_area_tiles(game);

    for (int i = 0; i < game->dialog.line_count; i++) {
        render_draw_text(1, VIEW_HEIGHT + 1 + i, game->dialog.lines[i]);
    }

    /* Tell the player there is more to read, rather than just stopping. */
    if (game->dialog.more) {
        render_draw_text(1, VIEW_HEIGHT + 1 + DIALOG_MAX_LINES, "-- more --");
    }
}

static void dialog_handle(Game *game, InputEvent event)
{
    if (event == INPUT_QUIT) {
        game->running = 0;
        return;
    }

    /* Any key turns the page; the last page ends the conversation. */
    if (event != INPUT_NONE) {
        if (!dialog_advance(&game->dialog)) {
            game->mode = MODE_EXPLORE;
        }
    }
}

/* ---- MENU ---- */

static void menu_draw(const Game *game)
{
    draw_area_tiles(game);

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        char line[32];
        if (i == 2) {
            snprintf(line, sizeof line, "%c %s: %s",
                     i == game->menu_index ? '>' : ' ', menu_items[i],
                     game->encounters_enabled ? "on" : "off");
        } else {
            snprintf(line, sizeof line, "%c %s",
                     i == game->menu_index ? '>' : ' ', menu_items[i]);
        }
        render_draw_text(1, VIEW_HEIGHT + 1 + i, line);
    }
}

static void menu_handle(Game *game, InputEvent event)
{
    switch (event) {
        case INPUT_UP:
            game->menu_index--;
            if (game->menu_index < 0) {
                game->menu_index = MENU_ITEM_COUNT - 1;
            }
            break;

        case INPUT_DOWN:
            game->menu_index++;
            if (game->menu_index >= MENU_ITEM_COUNT) {
                game->menu_index = 0;
            }
            break;

        case INPUT_CONFIRM:
            if (game->menu_index == 0) {
                char line[DIALOG_LINE_LEN];
                snprintf(line, sizeof line, "HP %d  MP %d  ATK %d  DEF %d",
                         game->player.hp, game->player.mp,
                         game->player.attack, game->player.defense);
                dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
                game->mode = MODE_DIALOG;
            } else if (game->menu_index == 1) {
                dialog_wrap(&game->dialog,
                            "Recover four fragments of the Crown of "
                            "Hollis. Payment on completion.",
                            TEXTBOX_WIDTH);
                game->mode = MODE_DIALOG;
            } else if (game->menu_index == 2) {
                /* Checkpoint C: the player can turn random encounters
                   off entirely, as the FF Pixel Remasters allow. */
                char line[DIALOG_LINE_LEN];

                game->encounters_enabled = !game->encounters_enabled;
                game->steps_since_encounter = 0;

                snprintf(line, sizeof line, "Random encounters are now %s.",
                         game->encounters_enabled ? "ON" : "OFF");

                dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
                game->mode = MODE_DIALOG;
            } else {
                game->mode = MODE_EXPLORE;
            }
            break;

        case INPUT_MENU:
            game->mode = MODE_EXPLORE;
            break;

        case INPUT_QUIT:
            game->running = 0;
            break;

        case INPUT_LEFT:
        case INPUT_RIGHT:
        case INPUT_FLEE:
        case INPUT_NONE:
            break;
    }
}


/* ---- BATTLE ---- */

static void battle_draw(const Game *game)
{
    const Battle *battle = &game->battle;

    render_draw_text(0, 0, "-- BATTLE --");

    for (int i = 0; i < battle->enemy_count; i++) {
        const Enemy *enemy = &battle->enemies[i];
        char line[48];

        if (enemy->alive) {
            snprintf(line, sizeof line, "%c %-20s %3d",
                     i == battle->target ? '>' : ' ',
                     enemy->type->name, enemy->hp);
        } else {
            snprintf(line, sizeof line, "  %-20s  --", enemy->type->name);
        }

        render_draw_text(0, 2 + i, line);
    }

    char status[48];
    snprintf(status, sizeof status, "%s  HP %d/%d  Lv %d",
             game->player.name[0] != '\0' ? "You" : "You",
             game->player.hp, game->player.max_hp, game->player.level);
    render_draw_text(0, 6, status);

    for (int i = 0; i < battle->log_count; i++) {
        render_draw_text(0, 8 + i, battle->log[i]);
    }

    render_draw_text(0, VIEW_HEIGHT + 1,
                     "Enter attack, w/s target, f flee");
}

static void battle_finish(Game *game)
{
    Battle *battle = &game->battle;
    char line[DIALOG_LINE_LEN];

    if (battle->outcome == BATTLE_WON) {
        int levels = party_gain_xp(&game->player, battle->xp_reward);
        game->player.gold += battle->gold_reward;

        if (levels > 0) {
            snprintf(line, sizeof line,
                     "Victory. %d XP, %d gold. Level %d now.",
                     battle->xp_reward, battle->gold_reward,
                     game->player.level);
        } else {
            snprintf(line, sizeof line, "Victory. %d XP and %d gold.",
                     battle->xp_reward, battle->gold_reward);
        }
    } else if (battle->outcome == BATTLE_FLED) {
        snprintf(line, sizeof line, "You got away.");
    } else {
        /* Defeat. Checkpoint C made the penalty configurable; for now
           the hero wakes at 1 HP, having lost nothing but dignity. */
        game->player.hp = 1;
        snprintf(line, sizeof line,
                 "You wake later, face down, somehow alive.");
    }

    dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
    game->mode = MODE_DIALOG;
}

static void battle_handle(Game *game, InputEvent event)
{
    Battle *battle = &game->battle;

    switch (event) {
        case INPUT_UP:
            battle_cycle_target(battle, -1);
            return;

        case INPUT_DOWN:
            battle_cycle_target(battle, 1);
            return;

        case INPUT_CONFIRM:
            battle_round(battle, &game->player, &game->rng, 0);
            break;

        case INPUT_FLEE:
            battle_round(battle, &game->player, &game->rng, 1);
            break;

        case INPUT_QUIT:
            game->running = 0;
            return;

        default:
            return;
    }

    if (battle->outcome != BATTLE_ONGOING) {
        battle_finish(game);
    }
}

/* ---- the dispatch table ---- */

static const ModeHandler mode_table[MODE_COUNT] = {
    [MODE_EXPLORE] = { "explore", explore_draw, explore_handle },
    [MODE_DIALOG]  = { "dialog",  dialog_draw,  dialog_handle  },
    [MODE_MENU]    = { "menu",    menu_draw,    menu_handle    },
    [MODE_BATTLE]  = { "battle",  battle_draw,  battle_handle  }
};

/* ---- public interface ---- */

Game *game_create(void)
{
    Game *game = malloc(sizeof *game);
    if (game == NULL) {
        fprintf(stderr, "game_create: out of memory\n");
        return NULL;
    }

    game->world = world_create();
    if (game->world == NULL) {
        free(game);
        return NULL;
    }

    game->player = entity_create_player();
    game->player.x = 2;
    game->player.y = 2;
    game->dialog.line_count = 0;
    game->mode = MODE_EXPLORE;
    game->menu_index = 0;
    game->running = 1;
    rng_seed(&game->rng, (uint32_t)time(NULL));
    game->steps_since_encounter = 0;
    game->encounters_enabled = 1;

    return game;
}

void game_destroy(Game *game)
{
    if (game == NULL) {
        return;
    }

    world_destroy(game->world);
    free(game);
}

void game_draw(const Game *game)
{
    render_clear();
    mode_table[game->mode].draw(game);
    render_present();
}

void game_handle(Game *game, InputEvent event)
{
    mode_table[game->mode].handle(game, event);
}

const char *game_mode_name(GameMode mode)
{
    return mode_table[mode].name;
}
