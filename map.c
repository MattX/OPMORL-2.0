/*
 *  map.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"


/*
 * Finds a random tile on the specified map level. If can_have_mon is false, the returned
 * tile will not have a monster on it. The tile will be of one of types specified in
 * tile_types.
 * Returns 0 if no empty tile is available, 1 otherwise. Stores the coordinates of the
 * empty tile in x and y.
 */
int find_floor_tile(int level, int *x, int *y, int tile_types, int can_have_mon)
{
    int avail[LEVEL_HEIGHT][LEVEL_WIDTH];
    int nb_avail = 0;
    int i_selected;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (lvl_map[level][i_x][i_y] & tile_types) {
                if (!can_have_mon && find_mon_at(level, i_x, i_y) != NULL)
                    avail[i_x][i_y] = 0;
                else {
                    avail[i_x][i_y] = 1;
                    nb_avail++;
                }
            } else
                avail[i_x][i_y] = 0;
        }
    }

    if (nb_avail == 0)
        return 0;

    i_selected = rand_int(0, nb_avail);
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (i_selected == 0) {
                *x = i_x;
                *y = i_y;
                return 1;
            }
            if (avail[i_x][i_y])
                i_selected--;
        }
    }

    // Should not reach here
    return 0;
}

void create_lvl(int level)
{
    int i, j;
    for (i = 0; i < LEVEL_HEIGHT; i++) {
        for (j = 0; j < LEVEL_WIDTH; j++) {
            if (i == 0 || j == 0 || i == LEVEL_HEIGHT - 1 || j == LEVEL_WIDTH - 1) {
                lvl_map[level][i][j] = T_WALL;
            } /* Around */
            else
                lvl_map[level][i][j] = T_FLOOR;
        }
    }

    lvl_map[level][rand_int(1, LEVEL_HEIGHT - 2)][rand_int(1, LEVEL_WIDTH - 2)] = T_STAIRS_UP;
}