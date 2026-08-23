#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "game.h"
#include "paths.h"
#include "map.h"
#include "render.h"
#include "combat_math.h"
#include "battle.h"
#include "party.h"
#include "inventory.h"
#include "config.h"
#include "magic.h"
#include "shop.h"
#include "save.h"

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

        /* A shopkeeper says their piece, then opens the stock list. */
        if (npc->shopkeeper &&
            shop_for_area((int)game->world->current) != NULL) {
            game->shop_index = 0;
            game->dialog_return = MODE_SHOP;
        } else {
            game->dialog_return = MODE_EXPLORE;
        }

        game->mode = MODE_DIALOG;
        return;
    }

    if (map_is_walkable(world_current_map(game->world), target_x, target_y)) {
        entity_move(&game->player, dx, dy);

        const Warp *warp = world_warp_at(game->world,
                                         game->player.x, game->player.y);
        if (warp != NULL) {
            /* A gated warp checks the inventory rather than a flag. The
               fragments the player is carrying are the progress record,
               which means saving them costs nothing extra -- they were
               already in the save file. */
            if (warp->requires_item >= 0 &&
                inventory_count_of(&game->inventory,
                                   (ItemId)warp->requires_item) == 0) {
                dialog_start(&game->dialog,
                             warp->locked_speech != NULL
                                 ? warp->locked_speech
                                 : "The way is closed.",
                             TEXTBOX_WIDTH);
                game->dialog_return = MODE_EXPLORE;
                game->mode = MODE_DIALOG;

                /* Step back off the door so the message is not repeated
                   on every keypress. */
                game->player.x -= dx;
                game->player.y -= dy;
                return;
            }

            game->world->current = warp->destination;
            game->player.x = warp->destination_x;
            game->player.y = warp->destination_y;
            game->steps_since_encounter = 0;
            return;
        }

        const BossTrigger *trigger = world_boss_at(game->world,
                                                   game->player.x,
                                                   game->player.y);
        if (trigger != NULL) {
            const BossType *boss = battle_boss((BossId)trigger->boss_id);

            /* Already beaten? Its reward is in the pack, so there is
               nothing here any more. */
            if (boss != NULL &&
                inventory_count_of(&game->inventory,
                                   (ItemId)boss->reward_item) == 0) {
                battle_begin_boss(&game->battle, (BossId)trigger->boss_id);
                game->mode = MODE_BATTLE;
                return;
            }
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
    game->shop_index = 0;
    game->dialog_return = MODE_EXPLORE;
                game->mode = MODE_ITEMS;
            } else if (game->menu_index == 2) {
                game->spell_index = 0;
                game->mode = MODE_SPELLS;
            } else if (game->menu_index == 3) {
                game->gear_slot = 0;
    game->shop_index = 0;
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

/* Defined with the rest of the ending, far below; declared here because
   winning the last fight is what starts it. */
static void ending_show_page(Game *game, int page);

static void battle_finish(Game *game)
{
    Battle *battle = &game->battle;
    char line[DIALOG_LINE_LEN];

    if (battle->outcome == BATTLE_WON) {
        int xp = (battle->xp_reward * game->config.xp_rate_pct) / 100;
        int levels = party_gain_xp(&game->player, xp);
        party_add_gold(&game->player, battle->gold_reward);

        /* A boss hands over what it was guarding. Vex was carrying two
           fragments, having been ahead of the player the whole way. */
        if (battle->boss_id >= 0) {
            const BossType *boss = battle_boss((BossId)battle->boss_id);

            if (boss != NULL) {
                inventory_add(&game->inventory, (ItemId)boss->reward_item, 1);

                if (battle->boss_id == BOSS_VEX) {
                    inventory_add(&game->inventory, ITEM_FRAGMENT_IV, 1);

                    ending_show_page(game, 0);
                    game->mode = MODE_ENDING;
                    return;
                }

                dialog_wrap(&game->dialog,
                            "It stops. Among what it leaves behind is a "
                            "piece of worked gold, heavier than it has "
                            "any right to be.",
                            TEXTBOX_WIDTH);
                game->dialog_return = MODE_EXPLORE;
                game->mode = MODE_DIALOG;
                return;
            }
        }

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

        /* The battle screen has said "m magic" since Chapter 18, and
           until now nothing here listened for it. Everything the spell
           menu needs to work mid-fight already existed; only the door
           was missing. */
        case INPUT_MENU:
            game->spell_index = 0;
            game->mode = MODE_SPELLS;
            return;

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
            /* Backing out of the spell list should return you to
               whatever you opened it from. */
            game->mode = (game->battle.outcome == BATTLE_ONGOING)
                       ? MODE_BATTLE : MODE_MENU;
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

    /* If the spell -- or the blow that answered it -- ended the fight,
       hand off to the same function an ordinary victory uses. Without
       this, killing the last enemy with magic awarded no XP, no gold,
       and no key item, and the final boss never triggered the ending.
       One exit path, used by every way a battle can end. */
    if (in_battle && game->battle.outcome != BATTLE_ONGOING) {
        battle_finish(game);
        return;
    }

    dialog_start(&game->dialog, ctx.message, TEXTBOX_WIDTH);
    game->dialog_return = in_battle ? MODE_BATTLE : MODE_EXPLORE;
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
    snprintf(stats, sizeof stats, "ATK %d  DEF %d  AGI %d  Gold %d",
             entity_attack(&game->player), entity_defense(&game->player),
             entity_agility(&game->player), game->player.gold);
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
        case INPUT_RIGHT:
            /* Gear is no longer free to swap: buying it in a shop is what
               equips it. The screen is now a read-only summary. */
            break;

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


/* ---- SHOP ---- */

static void shop_draw(const Game *game)
{
    const Shop *shop = shop_for_area((int)game->world->current);

    draw_area_tiles(game);

    if (shop == NULL) {
        render_draw_text(1, VIEW_HEIGHT + 1, "Nobody here is selling.");
        return;
    }

    char purse[48];
    snprintf(purse, sizeof purse, "Gold %d", game->player.gold);
    render_draw_text(1, VIEW_HEIGHT + 1, purse);

    /* Entry stock_count is the inn, listed after the goods. */
    int rows = shop->stock_count + (shop->inn_cost > 0 ? 1 : 0);

    int first = game->shop_index - 1;
    if (first < 0) {
        first = 0;
    }
    if (first > rows - 3) {
        first = rows - 3;
    }
    if (first < 0) {
        first = 0;
    }

    for (int i = first; i < rows && i < first + 3; i++) {
        char line[48];

        if (i < shop->stock_count) {
            const StockEntry *e = &shop->stock[i];
            snprintf(line, sizeof line, "%c %-17s %4d",
                     i == game->shop_index ? '>' : ' ',
                     shop_entry_name(e), shop_entry_price(e));
        } else {
            snprintf(line, sizeof line, "%c %-17s %4d",
                     i == game->shop_index ? '>' : ' ',
                     "A bed for the night", shop->inn_cost);
        }

        render_draw_text(1, VIEW_HEIGHT + 2 + (i - first), line);
    }
}

static void shop_confirm(Game *game)
{
    const Shop *shop = shop_for_area((int)game->world->current);
    if (shop == NULL) {
        game->mode = MODE_EXPLORE;
        return;
    }

    char reason[96];

    if (game->shop_index < shop->stock_count) {
        shop_buy(&shop->stock[game->shop_index], &game->player,
                 &game->inventory, reason, sizeof reason);
    } else if (shop_rest(shop, &game->player, reason, sizeof reason)) {
        /* Checkpoint E: inns are the save point. The save call lives
           here, in the caller that holds the whole Game, rather than
           inside shop_rest -- which knows only about a Shop and a
           Player, and should stay that way. */
        char save_reason[48];

        if (game_save(game, save_reason, sizeof save_reason)) {
            snprintf(reason, sizeof reason,
                     "You sleep. HP and MP restored, and recorded.");
        } else {
            snprintf(reason, sizeof reason, "You sleep, but: %s",
                     save_reason);
        }
    }

    dialog_start(&game->dialog, reason, TEXTBOX_WIDTH);
    game->dialog_return = MODE_SHOP;
    game->mode = MODE_DIALOG;
}

static void shop_handle(Game *game, InputEvent event)
{
    const Shop *shop = shop_for_area((int)game->world->current);
    int rows = (shop == NULL)
             ? 0
             : shop->stock_count + (shop->inn_cost > 0 ? 1 : 0);

    switch (event) {
        case INPUT_UP:
            if (rows > 0) {
                game->shop_index = (game->shop_index + rows - 1) % rows;
            }
            break;

        case INPUT_DOWN:
            if (rows > 0) {
                game->shop_index = (game->shop_index + 1) % rows;
            }
            break;

        case INPUT_CONFIRM:
            if (rows == 0) {
                game->mode = MODE_EXPLORE;
            } else {
                shop_confirm(game);
            }
            break;

        case INPUT_MENU:
            game->mode = MODE_EXPLORE;
            break;

        case INPUT_QUIT:
            game->running = 0;
            break;

        default:
            break;
    }
}

/* ---- the dispatch table ---- */

/* ---- ENDING ----

   The last thing the game does. Pages of text, advanced with confirm,
   and then the loop stops. Kept as a table of strings for the same
   reason the world is a table: the words are content, and content does
   not belong inside control flow. */
static const char *const ending_pages[] = {
    "The four pieces sit together on the flagstones, and do not look "
    "like a crown. They look like four pieces.",

    "Then they do look like a crown. There is no sound. The wards come "
    "back the way feeling comes back into a hand -- unpleasantly, and "
    "all at once.",

    "Perrin Culch is still on the step. He does not ask what happened. "
    "He writes something short in the ledger, closes it, and sits with "
    "his hands flat on the cover.",

    "Ammel files the closing report eleven days later. Under OUTCOME "
    "she writes: RECOVERED, COMPLETE. Under NOTES she writes nothing "
    "at all, which is the only kindness available to her.",

    "The completion bonus is forty gold. The Guild considers this "
    "generous, and says so, in writing.",

    "SOME ASSEMBLY REQUIRED -- A Quest in Four Pieces.  Thank you for "
    "playing."
};

#define ENDING_PAGE_COUNT ((int)(sizeof ending_pages / sizeof ending_pages[0]))

/* Opens the ending on a given page, reusing the Chapter 12 text box so
   long passages page properly instead of being silently cut at four
   lines. Writing this with dialog_wrap was a bug: it renders only the
   first page, and the rest of the sentence is simply lost. */
static void ending_show_page(Game *game, int page)
{
    game->ending_page = page;
    dialog_start(&game->dialog, ending_pages[page], TEXTBOX_WIDTH);
}

static void ending_draw(const Game *game)
{
    render_draw_text(1, 1, "THE CROWNLESS COURT");

    for (int i = 0; i < game->dialog.line_count; i++) {
        render_draw_text(1, 3 + i, game->dialog.lines[i]);
    }

    int last = (game->ending_page + 1 >= ENDING_PAGE_COUNT) &&
               !game->dialog.more;

    render_draw_text(1, 4 + DIALOG_MAX_LINES,
                     last ? "-- the end --" : "-- more --");
}

static void ending_handle(Game *game, InputEvent event)
{
    if (event == INPUT_NONE) {
        return;
    }

    /* Finish the current passage before moving to the next one. */
    if (dialog_advance(&game->dialog)) {
        return;
    }

    if (game->ending_page + 1 < ENDING_PAGE_COUNT) {
        ending_show_page(game, game->ending_page + 1);
        return;
    }

    game->running = 0;
}

static const ModeHandler mode_table[MODE_COUNT] = {
    [MODE_EXPLORE] = { "explore", explore_draw, explore_handle },
    [MODE_DIALOG]  = { "dialog",  dialog_draw,  dialog_handle  },
    [MODE_MENU]    = { "menu",    menu_draw,    menu_handle    },
    [MODE_BATTLE]  = { "battle",  battle_draw,  battle_handle  },
    [MODE_ITEMS]   = { "items",   items_draw,   items_handle   },
    [MODE_SPELLS]  = { "spells",  spells_draw,  spells_handle  },
    [MODE_GEAR]    = { "gear",    gear_draw,    gear_handle    },
    [MODE_SHOP]    = { "shop",    shop_draw,    shop_handle    },
    [MODE_ENDING]  = { "ending",  ending_draw,  ending_handle  }
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

    config_defaults(&game->config);
    {
        char path[512];
        paths_data(path, sizeof path, "config.txt");
        config_load(&game->config, path);
    }

    /* Content tables come from disk when they are there, and fall back
       to the built-in copies when they are not. Either way the game
       starts -- a missing data file is a downgrade, not a failure.

       This must happen BEFORE the hero is created, because creating a
       hero reads the level table for their starting stats. Loading it
       afterwards leaves a level 1 hero built from the old numbers. */
    {
        char reason[96];
        char path[512];

        paths_data(path, sizeof path, PARTY_LEVELS_FILE);
        if (!party_load_levels(path, reason, sizeof reason)) {
            fprintf(stderr, "levels: %s\n", reason);
        }

        paths_data(path, sizeof path, MAGIC_SPELLS_FILE);
        if (!magic_load_spells(path, reason, sizeof reason)) {
            fprintf(stderr, "spells: %s\n", reason);
        }
    }

    game->player = entity_create_player();
    game->player.x = 2;
    game->player.y = 2;

    /* There is no battle yet, and several places ask whether one is in
       progress before the first encounter ever happens. Leaving this
       struct uninitialised makes those reads undefined behaviour --
       Chapter 22's lesson, arriving again in Chapter 23's new code.

       Note that zeroing alone would be wrong: BATTLE_ONGOING is 0, so a
       memset would claim a fight is under way. Say "finished" out loud,
       exactly as battle_begin does for an area with no roster. */
    memset(&game->battle, 0, sizeof game->battle);
    game->battle.outcome = BATTLE_WON;
    game->battle.boss_id = -1;

    game->dialog.line_count = 0;
    game->mode = MODE_EXPLORE;
    game->menu_index = 0;
    game->running = 1;
    rng_seed(&game->rng, (uint32_t)time(NULL));
    inventory_init(&game->inventory);
    inventory_add(&game->inventory, ITEM_GUILD_COMMISSION, 1);
    inventory_add(&game->inventory, ITEM_BREAD_RATION, 3);

    game->item_index = 0;
    game->spell_index = 0;
    game->gear_slot = 0;
    game->shop_index = 0;
    game->dialog_return = MODE_EXPLORE;
    game->steps_since_encounter = 0;
    game->ending_page = 0;
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


/* ---- save and load ---- */

int game_save(Game *game, char *reason, size_t reason_size)
{
    SaveData data;
    memset(&data, 0, sizeof data);

    data.version = SAVE_VERSION;
    snprintf(data.name, sizeof data.name, "%s", game->player.name);
    data.area = (int)game->world->current;
    data.x = game->player.x;
    data.y = game->player.y;
    data.level = game->player.level;
    data.xp = game->player.xp;
    data.gold = game->player.gold;
    data.hp = game->player.hp;
    data.mp = game->player.mp;
    for (int s = 0; s < SLOT_COUNT; s++) {
        data.equipped[s] = game->player.equipped[s];
    }
    data.rng_state = game->rng.state;

    char path[512];
    paths_save(path, sizeof path);

    return save_write(path, &data, &game->inventory,
                      reason, reason_size);
}

int game_load(Game *game, char *reason, size_t reason_size)
{
    SaveData data;
    Inventory loaded;
    inventory_init(&loaded);

    char path[512];
    paths_save(path, sizeof path);

    if (!save_read(path, &data, &loaded, reason, reason_size)) {
        inventory_free(&loaded);
        return 0;
    }

    /* save_read already validated every field, so from here it is a
       straight copy -- no second guessing, no clamping that would hide a
       value the loader should have rejected. */
    snprintf(game->player.name, sizeof game->player.name, "%s", data.name);
    game->world->current = (AreaId)data.area;
    game->player.x = data.x;
    game->player.y = data.y;
    game->player.level = data.level;
    game->player.xp = data.xp;
    game->player.gold = data.gold;
    for (int s = 0; s < SLOT_COUNT; s++) {
        game->player.equipped[s] = data.equipped[s];
    }

    /* Derived stats come from the level table, never from the file, so a
       hand-edited save cannot invent a hero with 9999 attack. */
    party_apply_level(&game->player);

    game->player.hp = data.hp > game->player.max_hp
                    ? game->player.max_hp : data.hp;
    game->player.mp = data.mp > game->player.max_mp
                    ? game->player.max_mp : data.mp;

    rng_seed(&game->rng, data.rng_state);

    inventory_free(&game->inventory);
    game->inventory = loaded;

    game->player.status = STATUS_NONE;
    game->player.status_turns = 0;

    return 1;
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
