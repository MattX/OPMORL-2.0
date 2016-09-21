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
        if (find_mon_at(to_dlvl, new_x, new_y))
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