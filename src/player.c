/*
 *  player.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 21/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"
#include "linkedlist.h"


/**
 * Compute the level to which you fall if you jump into the collapsed floor at
 * the given coordinates
 * @param dlvl, at Player coordinates
 * @return The fall level
 */
static int get_fall_level(int dlvl, Coord at)
{
    int fall_dlvl = dlvl + 1;
    while (fall_dlvl < DLVL_MAX && maps[fall_dlvl][at.x][at.y] == T_COLLAPSED) {
        fall_dlvl++;
    }

    return fall_dlvl;
}


/**
 * Moves the player, checking that the target cell can be walked on and is
 * inside the level, and that no monster stands in the way
 * @param to Destination coordinates
 * @return The number of turns the action took
 */
int move_rodney(Coord to)
{
    Monster *target;

    if (to.x < 0 || to.y < 0 || to.x >= LEVEL_HEIGHT || to.y >= LEVEL_WIDTH)
        return 0;

    if ((target = find_mon_at(rodney.dlvl, to)) != NULL) {
        if (target->flags & MF_INVISIBLE) {
            pline("Wait! There's a %s there!", target->type->name);
            target->flags &= ~MF_INVISIBLE;
            return 1;
        } else {
            return rodney_attacks(target, false);
        }
    }

    TileType target_type = maps[rodney.dlvl][to.x][to.y];

    if (!IS_WALKABLE(target_type)) {
        if (target_type == T_DOOR_CLOSED) {
            return toggle_door(
                    (Coord) {to.x - rodney.pos.x, to.y - rodney.pos.y}, true);
        } else if (target_type == T_COLLAPSED) {
            int fall_level = get_fall_level(rodney.dlvl, to);

            bool confirm = yes_no("The floor has collapsed here, and you can "
                                          "see the %d levels below. Jump?",
                                  fall_level);
            if (!confirm) {
                return 0;
            }

            pline("You fall through the collapsed floor.");

            if (change_dlvl(fall_level, -1))
                take_damage((rodney.dlvl - fall_level) * 3);
        } else if (target_type == T_PORTCULLIS_DOWN) {
            pline("You bump into the lowered portcullis.");
            return 0;
        } else
            return 0;
    }

    rodney.pos = to;
    return 1;
}


/**
 * Changes the player level.
 * @param to_dlvl The depth level the player should be switched to
 * @param tile_type The tile type on which the player should land. Should be
 * T_STAIRS_UP or T_STAIRS_DOWN, or -1, in which case the player will land
 * close to its current position.
 * @return The number of turns the action took
 */
int change_dlvl(int to_dlvl, int tile_type)
{
    Coord new;
    int found_tile;

    if (to_dlvl < 0 || to_dlvl >= DLVL_MAX) {
        pline("A mysterious force prevents you from passing");
        return 0;
    }

    if (tile_type < 0 || tile_type > NB_TILE_TYPES) {
        found_tile = find_closest(to_dlvl, &new, false, T_FLOOR, rodney.pos);
    } else {
        found_tile = find_tile(&new, to_dlvl, false, tile_type);
    }

    if (!found_tile && god_mode) {
        // Make an extra effort to find a tile
        pline("Made an extra effort for a God.");
        found_tile = find_tile(&new, to_dlvl, false, -1);
    }

    if (!found_tile) {
        if (tile_type < 0 || tile_type > NB_TILE_TYPES) {
            pline("You are kicked back up by a mysterious force.");
        } else {
            if (find_mon_at(to_dlvl, new))
                pline("The staircase is blocked by a monster.");
            else // There is no corresponding staircase
                pline("The staircase is blocked by debris from its collapsed "
                              "roof.");
        }
        return 0;
    }

    rodney.dlvl = to_dlvl;
    rodney.pos = new;

    if (!visited[rodney.dlvl]) {
        visited[rodney.dlvl] = true;
        make_monsters(rodney.dlvl, -1);
    }

    return 1;
}

/**
 * Performs necessary checks, then move the player up or down if he is on the
 * correct staircase.
 * @param up True if the player pressed the up (<) key, false otherwise.
 * @return The number of turns the action took
 */
int use_stairs(bool up)
{
    if (up) {
        if (maps[rodney.dlvl][rodney.pos.x][rodney.pos.y] != T_STAIRS_UP &&
            !god_mode) {
            pline("You can't go up here!");
            return 0;
        }
        if (rodney.dlvl == 0) {
            int confirm = yes_no(
                    "Leave the dungeon? You have %d gold pieces and %d points.",
                    rodney.gold,
                    rodney.score);
            if (confirm)
                exit_game();
            else
                pline("Ok.");

            return 0;
        } else {
            return change_dlvl(rodney.dlvl - 1, T_STAIRS_DOWN);
        }
    } else {
        if (maps[rodney.dlvl][rodney.pos.x][rodney.pos.y] != T_STAIRS_DOWN &&
            !god_mode) {
            pline("You can't go down here!");
            return 0;
        } else {
            return change_dlvl(rodney.dlvl + 1, T_STAIRS_UP);
        }
    }
}

void add_permanent_effect(MixinType mixin)
{
    for (int i = 0; i < MAX_MIXIN; i++) {
        if (rodney.permanent_effects[i] == mixin)
            return;
        if (rodney.permanent_effects[i] == MT_NONE)
            rodney.permanent_effects[i] = mixin;
    }
}

bool has_permanent_effect(MixinType mixin)
{
    for (int i = 0; i < MAX_MIXIN; i++) {
        if (rodney.permanent_effects[i] == mixin)
            return true;
    }

    return false;
}


/**
 * Update target to suffer the effects of a player attack.
 * @param target The targeted monster
 * @param melee true if the attack was done through a melee attack, false if
 * the attack was ranged. If true, the target should be adjacent to the
 * player.
 */
int rodney_attacks(Monster *target, bool melee)
{
    if (ndn(2, 10) > 4 - target->type->ac - rodney.explevel) {
        int damage = 1;
        if (rodney.wielded != NULL) {
            if (melee && rodney.wielded->type->class->o_class_flag & OT_MELEE) {
                damage += ndn(2, rodney.wielded->type->power) / 20;
                damage += ndn((1 + rodney.explevel / 10),
                              2 * rodney.wielded->enchant);
            } else if (!melee &&
                       rodney.wielded->type->class->o_class_flag & OT_WAND) {
                damage += ndn(2, rodney.wielded->type->power) / 20;
            } else {
                damage = 2;
            }
            target->hp -= damage;
#ifdef DEBUG
            pline("Hit for %d damage, remaining %d/%d", damage, target->hp,
                  target->type->max_hp);
#endif
        } else {
            target->hp -= 1;
        }

        if (check_dead(target, true)) {
            pline("You kill the %s.", target->type->name);
        } else {
            pline("You hit the %s.", target->type->name);
        }
    } else
        pline("You miss the %s.", target->type->name);

    return 1;
}


/**
 * Returns true if the effect applies to the player either
 * because it's in the inventory or because it's a permanent effect.
 */
bool player_has_effect(MixinType effect)
{
    return has_inventory_effect(effect) || has_permanent_effect(effect);
}


/**
 * Regenerate Rodney's HP if required.
 */
void regain_hp()
{
    static int turns_since_regain = 0;

    if ((turns_since_regain > 10 ||
         (player_has_effect(MT_BG_REGEN) && turns_since_regain > 5)) &&
        rodney.max_hp > rodney.hp) {
        rodney.hp += rodney.explevel;
        turns_since_regain = 0;
    } else {
        turns_since_regain++;
    }
}


/**
 * Takes damage and checks appropriate actions
 */
void take_damage(int damage)
{
    rodney.hp -= damage;

    if (rodney.hp < 0) {
        pline("You die...");
#ifndef DEBUG
        exit_game();
#endif
    }
}


void gain_exp(int exp, MagicClassTag class)
{
    rodney.magic_class_exp[class] += exp;
    if (rodney.magic_class_exp >= 0);
}


/**
 * Zaps a wand
 * @return The number of turns the action took
 */
int zap()
{
    Coord target_pos;
    Coord effective_pos;
    bool confirm;

    Object *weapon = select_from_inv(0);
    Monster *target;

    if (weapon == NULL) {
        pline("Never mind.");
        return 0;
    }

    pline("Zap where?");
    confirm = get_point(&target_pos);
    if (!confirm) {
        pline("Never mind.");
        return 0;
    }

    if (!is_visible(rodney.dlvl, rodney.pos, target_pos, &effective_pos,
                    true)) {
        target_pos = effective_pos;
    }

    target = find_mon_at(rodney.dlvl, target_pos);

    if (target == NULL) {
        pline("The ray hits the wall.");
        return 1;
    }

    rodney_attacks(target, false);

    return 1;
}


/**
 * Opens or closes a door
 * @param direction If not set to (0,0), the direction in which to open a door.
 * Otherwise, will prompt the player for a direction.
 * @param open If set to false, will close the door.
 * @return The number of turns the action took
 */
int toggle_door(Coord direction, bool open)
{
    TileType prev, new;

    if (direction.x == 0 && direction.y == 0)
        direction = get_direction();
    Coord target = coord_add(rodney.pos, direction);

    prev = open ? T_DOOR_CLOSED : T_DOOR_OPEN;
    new = open ? T_DOOR_OPEN : T_DOOR_CLOSED;

    if (maps[rodney.dlvl][target.x][target.y] == new) {
        pline("This door is already %s!", open ? "open" : "closed");
        return 0;
    } else if (maps[rodney.dlvl][target.x][target.y] != prev) {
        pline("You see no door there.");
        return 0;
    }

    if (ndn(2, 3) < 4) {
        pline("The door resists.");
    } else {
        pline("The door %s.", open ? "opens" : "closes");
        maps[rodney.dlvl][target.x][target.y] = new;
    }

    return 1;
}


/**
 * Helper function to set a portcullis' state
 * @param dlvl, pos The coordinates of the portcullis
 * @param open If true, raises the portcullis. Otherwise, lowers it.
 */
static void set_portcullis(int dlvl, Coord pos, bool open)
{
    TileType prev = open ? T_PORTCULLIS_DOWN : T_PORTCULLIS_UP;
    TileType new = open ? T_PORTCULLIS_UP : T_PORTCULLIS_DOWN;

    if (maps[dlvl][pos.x][pos.y] == prev) {
        maps[dlvl][pos.x][pos.y] = new;
        for (int i_neighbor = 0; i_neighbor < 4; i_neighbor++) {
            Coord neighbor = get_neighbor(pos, i_neighbor);
            if (valid_coordinates(neighbor.x, neighbor.y))
                set_portcullis(dlvl, neighbor, open);
        }
    }
}


/**
 * Toggle a portcullis
 * @param dlvl, pos The coordinates of the portcullis
 */
static void toggle_portcullis(int dlvl, Coord pos)
{
    bool open;

    if (maps[dlvl][pos.x][pos.y] == T_PORTCULLIS_DOWN)
        open = true;
    else if (maps[dlvl][pos.x][pos.y] == T_PORTCULLIS_UP)
        open = false;
    else {
        pline("You don't hear anything");
        return;
    }

    set_portcullis(dlvl, pos, open);
    pline("You hear a lound grinding sound.");
}


/**
 * Toggles a lever (duh)
 * @return The number of turns the action took
 */
int toggle_lever()
{
    if (maps[rodney.dlvl][rodney.pos.x][rodney.pos.y] != T_LEVER) {
        pline("You see no lever to toggle here.");
        return 0;
    }

    LinkedListNode *lever_descriptor = lever_connections->head;
    LeverConnection *lc = NULL;
    while (lever_descriptor != NULL) {
        lc = (LeverConnection *) lever_descriptor->element;
        if (lc->dlvl == rodney.dlvl && lc->pos.x == rodney.pos.x &&
            lc->pos.y == rodney.pos.y)
            break;

        lever_descriptor = lever_descriptor->next;
    }

    if (lever_descriptor == NULL) {
        if (dlvl_types[rodney.dlvl] == DLVL_ADMINISTRATOR) {
            // On the administrator level, an unregistered level is probably the
            // administrator's.
            pline("You hear a the sound of a gigantic pump starting up.");
            for (int i = 0; i < DLVL_MAX; i++) {
                dlvl_flags[i] &= ~DFLAGS_FLOODED;
            }
        } else
            pline("You hear a mechanism click, but nothing seems to happen.");
        return 1;
    }

    switch (maps[lc->target_dlvl][lc->target_pos.x][lc->target_pos.y]) {
    case T_PORTCULLIS_DOWN:
    case T_PORTCULLIS_UP:
        toggle_portcullis(lc->target_dlvl, lc->target_pos);
        break;
    default:
        pline("You hear cogs click, but nothing seems to happen.");
    }

    return 1;
}