#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "map.h"
#include "render.h"
#include "combat_math.h"
#include "battle.h"
#include "party.h"
#include "inventory.h"
#include "config.h"
#include "magic.h"

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
    "Items",
    "Magic",
    "Gear",
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
        game->dialog_return = MODE_EXPLORE;
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

    /* Any key turns the page; the last page hands control back to
       whichever mode opened the text box -- a spell cast mid-fight must
       return to the fight, not dump the player onto the map. */
    if (event != INPUT_NONE) {
        if (!dialog_advance(&game->dialog)) {
            game->mode = game->dialog_return;
            game->dialog_return = MODE_EXPLORE;
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
                snprintf(line, sizeof line, "HP %d/%d MP %d/%d ATK %d DEF %d",
                         game->player.hp, game->player.max_hp,
                         game->player.mp, game->player.max_mp,
                         entity_attack(&game->player),
                         entity_defense(&game->player));
                dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
                game->dialog_return = MODE_EXPLORE;
                game->mode = MODE_DIALOG;
            } else if (game->menu_index == 1) {
                game->item_index = 0;
    game->spell_index = 0;
    game->gear_slot = 0;
    game->dialog_return = MODE_EXPLORE;
                game->mode = MODE_ITEMS;
            } else if (game->menu_index == 2) {
                game->spell_index = 0;
                game->mode = MODE_SPELLS;
            } else if (game->menu_index == 3) {
                game->gear_slot = 0;
                game->mode = MODE_GEAR;
            } else if (game->menu_index == 4) {
                /* Checkpoint C: the player can turn random encounters
                   off entirely, as the FF Pixel Remasters allow. */
                char line[DIALOG_LINE_LEN];

                game->encounters_enabled = !game->encounters_enabled;
                game->steps_since_encounter = 0;

                snprintf(line, sizeof line, "Random encounters are now %s.",
                         game->encounters_enabled ? "ON" : "OFF");

                dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
                game->dialog_return = MODE_EXPLORE;
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
                     "Enter attack, m magic, w/s target, f flee");
}

static void battle_finish(Game *game)
{
    Battle *battle = &game->battle;
    char line[DIALOG_LINE_LEN];

    if (battle->outcome == BATTLE_WON) {
        int xp = (battle->xp_reward * game->config.xp_rate_pct) / 100;
        int levels = party_gain_xp(&game->player, xp);
        game->player.gold += battle->gold_reward;

        if (levels > 0) {
            snprintf(line, sizeof line,
                     "Victory. %d XP, %d gold. Level %d now.",
                     xp, battle->gold_reward, game->player.level);
        } else {
            snprintf(line, sizeof line, "Victory. %d XP and %d gold.",
                     xp, battle->gold_reward);
        }
    } else if (battle->outcome == BATTLE_FLED) {
        snprintf(line, sizeof line, "You got away.");
    } else {
        /* Defeat. Checkpoint C made the penalty configurable. */
        int lost = (game->player.gold * game->config.gold_loss_pct) / 100;

        game->player.gold -= lost;
        game->player.hp = 1;

        if (lost > 0) {
            snprintf(line, sizeof line,
                     "You wake face down, %d gold lighter.", lost);
        } else {
            snprintf(line, sizeof line,
                     "You wake later, face down, somehow alive.");
        }
    }

    dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
    game->dialog_return = MODE_EXPLORE;
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
            battle_round(battle, &game->player, &game->rng, 0,
                         game->config.enemy_damage_pct);
            break;

        case INPUT_FLEE:
            battle_round(battle, &game->player, &game->rng, 1,
                         game->config.enemy_damage_pct);
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


/* ---- ITEMS ---- */

static void items_draw(const Game *game)
{
    const Inventory *inv = &game->inventory;

    draw_area_tiles(game);

    if (inv->count == 0) {
        render_draw_text(1, VIEW_HEIGHT + 1, "You are carrying nothing.");
        render_draw_text(0, VIEW_HEIGHT + 3, "m/Enter close");
        return;
    }

    for (int i = 0; i < inv->count && i < 4; i++) {
        const ItemType *type = item_type(inv->stacks[i].id);
        char line[48];

        snprintf(line, sizeof line, "%c %-18s x%d%s",
                 i == game->item_index ? '>' : ' ',
                 type->name, inv->stacks[i].quantity,
                 type->key_item ? " (key)" : "");

        render_draw_text(1, VIEW_HEIGHT + 1 + i, line);
    }
}

static void items_use_selected(Game *game)
{
    Inventory *inv = &game->inventory;

    if (game->item_index < 0 || game->item_index >= inv->count) {
        return;
    }

    ItemId id = inv->stacks[game->item_index].id;
    const ItemType *type = item_type(id);
    char line[DIALOG_LINE_LEN];

    if (type->key_item) {
        snprintf(line, sizeof line, "The %s is not for using.", type->name);
        dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
        game->dialog_return = MODE_EXPLORE;
        game->mode = MODE_DIALOG;
        return;
    }

    switch (type->effect) {
        case ITEM_EFFECT_HEAL_HP: {
            /* Refuse rather than waste the item -- healing at full HP
               would consume it for nothing, which players rightly read
               as the game stealing from them. */
            if (game->player.hp >= game->player.max_hp) {
                snprintf(line, sizeof line, "You are not hurt.");
                break;
            }

            int before = game->player.hp;

            game->player.hp += type->magnitude;
            if (game->player.hp > game->player.max_hp) {
                game->player.hp = game->player.max_hp;
            }

            int healed = game->player.hp - before;
            inventory_remove_one(inv, id);

            snprintf(line, sizeof line, "%s restores %d HP.",
                     type->name, healed);
            break;
        }

        case ITEM_EFFECT_HEAL_MP: {
            if (game->player.mp >= game->player.max_mp) {
                snprintf(line, sizeof line, "Your head is already clear.");
                break;
            }

            int before = game->player.mp;

            game->player.mp += type->magnitude;
            if (game->player.mp > game->player.max_mp) {
                game->player.mp = game->player.max_mp;
            }

            int restored = game->player.mp - before;
            inventory_remove_one(inv, id);

            snprintf(line, sizeof line, "%s restores %d MP.",
                     type->name, restored);
            break;
        }

        default:
            snprintf(line, sizeof line, "Nothing happens.");
            break;
    }

    /* Removing an item can shrink the list under the cursor. */
    if (game->item_index >= inv->count) {
        game->item_index = inv->count > 0 ? inv->count - 1 : 0;
    }

    dialog_wrap(&game->dialog, line, TEXTBOX_WIDTH);
    game->dialog_return = MODE_EXPLORE;
    game->mode = MODE_DIALOG;
}

static void items_handle(Game *game, InputEvent event)
{
    int count = game->inventory.count;

    switch (event) {
        case INPUT_UP:
            if (count > 0) {
                game->item_index = (game->item_index + count - 1) % count;
            }
            break;

        case INPUT_DOWN:
            if (count > 0) {
                game->item_index = (game->item_index + 1) % count;
            }
            break;

        case INPUT_CONFIRM:
            if (count == 0) {
                game->mode = MODE_MENU;
            } else {
                items_use_selected(game);
            }
            break;

        case INPUT_MENU:
            game->mode = MODE_MENU;
            break;

        case INPUT_QUIT:
            game->running = 0;
            break;

        default:
            break;
    }
}


/* ---- SPELLS ---- */

static void spells_draw(const Game *game)
{
    int known = magic_known_count(game->player.level);

    draw_area_tiles(game);

    if (known == 0) {
        render_draw_text(1, VIEW_HEIGHT + 1, "You know no magic yet.");
        return;
    }

    char header[48];
    snprintf(header, sizeof header, "MP %d/%d",
             game->player.mp, game->player.max_mp);
    render_draw_text(1, VIEW_HEIGHT + 1, header);

    /* Show a window of three around the cursor, so a long spell list
       still fits under the map. */
    int first = game->spell_index - 1;
    if (first < 0) {
        first = 0;
    }
    if (first > known - 3) {
        first = known - 3;
    }
    if (first < 0) {
        first = 0;
    }

    for (int i = first; i < known && i < first + 3; i++) {
        const Spell *s = magic_spell_at(i);
        char line[48];

        snprintf(line, sizeof line, "%c %-13s %2d MP",
                 i == game->spell_index ? '>' : ' ', s->name, s->mp_cost);
        render_draw_text(1, VIEW_HEIGHT + 2 + (i - first), line);
    }
}

static void spells_cast_selected(Game *game)
{
    const Spell *spell = magic_spell_at(game->spell_index);
    if (spell == NULL) {
        return;
    }

    SpellContext ctx;
    ctx.caster = &game->player;
    ctx.rng = &game->rng;
    ctx.message[0] = '\0';
    ctx.target_hp = NULL;
    ctx.target_status = NULL;
    ctx.target_turns = NULL;

    if (game->mode == MODE_SPELLS && game->battle.outcome == BATTLE_ONGOING) {
        Enemy *e = &game->battle.enemies[game->battle.target];
        if (e->alive) {
            ctx.target_hp = &e->hp;
            ctx.target_status = &e->status;
            ctx.target_turns = &e->status_turns;
        }
    }

    int in_battle = (game->battle.outcome == BATTLE_ONGOING);
    int cast = magic_cast(spell, &ctx);

    /* Casting spends the hero's action, so the enemies answer -- but
       only if the spell actually went off. */
    if (cast && in_battle) {
        battle_enemy_turn(&game->battle, &game->player, &game->rng,
                          game->config.enemy_damage_pct);
    }

    dialog_start(&game->dialog, ctx.message, TEXTBOX_WIDTH);
    game->dialog_return = (in_battle && game->battle.outcome == BATTLE_ONGOING)
                        ? MODE_BATTLE : MODE_EXPLORE;
    game->mode = MODE_DIALOG;
}

static void spells_handle(Game *game, InputEvent event)
{
    int known = magic_known_count(game->player.level);

    switch (event) {
        case INPUT_UP:
            if (known > 0) {
                game->spell_index = (game->spell_index + known - 1) % known;
            }
            break;

        case INPUT_DOWN:
            if (known > 0) {
                game->spell_index = (game->spell_index + 1) % known;
            }
            break;

        case INPUT_CONFIRM:
            if (known == 0) {
                game->mode = MODE_MENU;
            } else {
                spells_cast_selected(game);
            }
            break;

        case INPUT_MENU:
            game->mode = MODE_MENU;
            break;

        case INPUT_QUIT:
            game->running = 0;
            break;

        default:
            break;
    }
}

/* ---- GEAR ---- */

static const char *const slot_names[SLOT_COUNT] = {
    "Weapon", "Armour", "Charm"
};

static void gear_draw(const Game *game)
{
    draw_area_tiles(game);

    for (int s = 0; s < SLOT_COUNT; s++) {
        const Gear *g = gear_at((GearSlot)s, game->player.equipped[s]);
        char line[48];

        snprintf(line, sizeof line, "%c %-7s %s",
                 s == game->gear_slot ? '>' : ' ',
                 slot_names[s], g != NULL ? g->name : "(none)");
        render_draw_text(1, VIEW_HEIGHT + 1 + s, line);
    }

    char stats[48];
    snprintf(stats, sizeof stats, "ATK %d  DEF %d  AGI %d",
             entity_attack(&game->player), entity_defense(&game->player),
             entity_agility(&game->player));
    render_draw_text(1, VIEW_HEIGHT + 1 + SLOT_COUNT, stats);
}

static void gear_handle(Game *game, InputEvent event)
{
    switch (event) {
        case INPUT_UP:
            game->gear_slot = (game->gear_slot + SLOT_COUNT - 1) % SLOT_COUNT;
            break;

        case INPUT_DOWN:
            game->gear_slot = (game->gear_slot + 1) % SLOT_COUNT;
            break;

        case INPUT_LEFT:
        case INPUT_RIGHT: {
            /* Cycle through what the hero owns for this slot. Buying is
               Chapter 19's job; for now everything is available so the
               stat maths can be seen working. */
            int count = gear_count((GearSlot)game->gear_slot);
            int step = (event == INPUT_RIGHT) ? 1 : count - 1;

            game->player.equipped[game->gear_slot] =
                (game->player.equipped[game->gear_slot] + step) % count;
            break;
        }

        case INPUT_MENU:
        case INPUT_CONFIRM:
            game->mode = MODE_MENU;
            break;

        case INPUT_QUIT:
            game->running = 0;
            break;

        default:
            break;
    }
}

/* ---- the dispatch table ---- */

static const ModeHandler mode_table[MODE_COUNT] = {
    [MODE_EXPLORE] = { "explore", explore_draw, explore_handle },
    [MODE_DIALOG]  = { "dialog",  dialog_draw,  dialog_handle  },
    [MODE_MENU]    = { "menu",    menu_draw,    menu_handle    },
    [MODE_BATTLE]  = { "battle",  battle_draw,  battle_handle  },
    [MODE_ITEMS]   = { "items",   items_draw,   items_handle   },
    [MODE_SPELLS]  = { "spells",  spells_draw,  spells_handle  },
    [MODE_GEAR]    = { "gear",    gear_draw,    gear_handle    }
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
    config_defaults(&game->config);
    config_load(&game->config, "assets/config.txt");

    inventory_init(&game->inventory);
    inventory_add(&game->inventory, ITEM_GUILD_COMMISSION, 1);
    inventory_add(&game->inventory, ITEM_BREAD_RATION, 3);

    game->item_index = 0;
    game->spell_index = 0;
    game->gear_slot = 0;
    game->dialog_return = MODE_EXPLORE;
    game->steps_since_encounter = 0;
    game->encounters_enabled = game->config.encounters;

    return game;
}

void game_destroy(Game *game)
{
    if (game == NULL) {
        return;
    }

    inventory_free(&game->inventory);
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
