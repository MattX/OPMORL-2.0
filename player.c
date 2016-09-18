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

    if (lvl_map[rodney.level][newx][newy] & (T_WALL | T_CLOSED_DOOR))
        return 0;

	rodney.posx = newx;
	rodney.posy = newy;
    return 1;
}

int use_stairs(int up)
{
    if (up)
        return 0;
    else {
        if (lvl_map[rodney.level][rodney.posy - 1][rodney.posx] != T_STAIRS_UP) {
            pline("You can't go down here !");
            return 0;
        }
    }

    return 1;
}
