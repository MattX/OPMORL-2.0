/*
 *  player.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 21/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"

/**
 * Moves the player, checking that the target cell can be walked on and is
 * inside the level (but no monster presence checks are performed.
 * @param to Destination coordinates
 * @return The number of turns the action took
 */
int move_rodney(Coord to)
{
    if (to.x < 0 || to.y < 0 || to.x >= LEVEL_HEIGHT || to.y >= LEVEL_WIDTH)
        return 0;

    if (!IS_WALKABLE(maps[rodney.dlvl][to.x][to.y]))
        return 0;

    rodney.pos = to;
    return 1;
}


/**
 * Changes the player level.
 * @param to_dlvl The depth level the player should be switched to
 * @param tile_type The tile type on which the player should land. Should be
 * T_STAIRS_UP or T_STAIRS_DOWN.
 * @return The number of turns the action took
 */
int change_dlvl_stairs(int to_dlvl, int tile_type)
{
    Coord new;

    if (!find_tile(to_dlvl, &new, false, tile_type)) {
        if (find_mon_at(to_dlvl, new))
            pline("The staircase is blocked by a monster.");
        else // There is no up staircase on the level below
            pline("The staircase is blocked by debris from its collapsed roof.");
        return 0;
    } else {
        rodney.dlvl = to_dlvl;
        rodney.pos = new;

        if (!visited[rodney.dlvl]) {
            visited[rodney.dlvl] = true;
            make_monsters(rodney.dlvl, -1);
        }

        return 1;
    }
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
        if (maps[rodney.dlvl][rodney.pos.x][rodney.pos.y] != T_STAIRS_UP) {
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
                pline("Ok");
        } else {
            return change_dlvl_stairs(rodney.dlvl - 1, T_STAIRS_DOWN);
        }
    } else {
        if (maps[rodney.dlvl][rodney.pos.x][rodney.pos.y] != T_STAIRS_DOWN) {
            pline("You can't go down here!");
            return 0;
        } else {
            return change_dlvl_stairs(rodney.dlvl + 1, T_STAIRS_UP);
        }
    }

    // Should not reach here
    return 0;
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


int open()
{
    Coord direction = get_direction();
    Coord target = coord_add(rodney.pos, direction);

    if (maps[rodney.dlvl][target.x][target.y] == T_OPEN_DOOR) {
        pline("This door is already open!");
        return 0;
    } else if (maps[rodney.dlvl][target.x][target.y] != T_CLOSED_DOOR) {
        pline("You see no door there.");
        return 0;
    }

    if (ndn(2, 3) < 4) {
        pline("The door resists.");
    } else {
        pline("The door opens.");
        maps[rodney.dlvl][target.x][target.y] = T_OPEN_DOOR;
    }

    return 1;
}