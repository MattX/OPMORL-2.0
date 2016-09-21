/*
 *  player.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 21/11/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "opmorl.h"

int move_rodney(int newx, int newy)
{
    if (newx < 0 || newy < 0 || newx >= LEVEL_HEIGHT || newy >= LEVEL_WIDTH)
        return 0;

    if (lvl_map[rodney.dlvl][newx][newy] & ~T_WALKABLE)
        return 0;

	rodney.posx = newx;
	rodney.posy = newy;
    return 1;
}

int change_dlvl(int to_dlvl, int place_on)
{
    int new_x, new_y;

    if (!find_floor_tile(to_dlvl, &new_x, &new_y, place_on, 0)) {
        if (find_mon_at(new_y, to_dlvl, new_x))
            pline("The staircase is blocked by a monster.");
        else // There is no up staircase on the level below
            pline("The staircase is blocked by debris from its collapsed roof.");
        return 0;
    } else {
        rodney.dlvl = to_dlvl;
        rodney.posx = new_x;
        rodney.posy = new_y;

        if (!visited[rodney.dlvl])
            make_monsters(rodney.dlvl, -1);

        return 1;
    }
}

int use_stairs(int up)
{
    if (up) {
        if (lvl_map[rodney.dlvl][rodney.posx][rodney.posy] != T_STAIRS_UP) {
            pline("You can't go up here!");
            return 0;
        }
        if (rodney.dlvl == 0) {
            int confirm = yes_no("Leave the dungeon? You have %d gold pieces and %d points.", rodney.gold,
                                 rodney.score);
            if (confirm)
                exit_game();
            else
                pline("Ok");
        } else {
            return change_dlvl(rodney.dlvl - 1, T_STAIRS_DOWN);
        }
    }
    else {
        if (lvl_map[rodney.dlvl][rodney.posx][rodney.posy] != T_STAIRS_DOWN) {
            pline("You can't go down here!");
            return 0;
        } else {
            return change_dlvl(rodney.dlvl + 1, T_STAIRS_UP);
        }
    }

    // Should not reach here
    return 0;
}

void add_permanent_effect(Mixin_type mixin)
{
    for (int i = 0; i < MAX_MIXIN; i++) {
        if (rodney.permanent_effects[i] == mixin)
            return;
        if (rodney.permanent_effects[i] == MT_NONE)
            rodney.permanent_effects[i] = mixin;
    }
}

bool has_permanent_effect(Mixin_type mixin)
{
    for (int i = 0; i < MAX_MIXIN; i++) {
        if (rodney.permanent_effects[i] == mixin)
            return true;
    }

    return false;
}

int rodney_attacks(Monster *target)
{
    if (ndn(2, 10) > 10 + target->type->ac - rodney.explevel) {
        int damage = 1;
        if (rodney.wielded != NULL) {
            if (rodney.wielded->type->class->o_class_flag &
                (OT_MELEE | OT_WAND)) {
                damage += ndn(2, rodney.wielded->type->power) / 20;
                damage += ndn((1 + rodney.explevel / 10),
                              2 * rodney.wielded->enchant);
                target->hp -= damage;
            } else if (rodney.wielded->type->class->o_class_flag & OT_TOOL) {
                target->hp -= 4;
            }
        } else {
            target->hp -= 1;
        }
        check_dead(target, true);
    } else
        pline("You miss the %s", target->type->name);

    return 1;
}