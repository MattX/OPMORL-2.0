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
 * Returns the number of available tiles satisfying the constraints. Stores the coordinates of the
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

    i_selected = rand_int(0, nb_avail - 1);
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (avail[i_x][i_y]) {
                if (i_selected == 0) {
                    *x = i_x;
                    *y = i_y;
                    return nb_avail;
                }
                i_selected--;
            }
        }
    }

    // Should not reach here
    return 0;
}

void create_level(int level)
{
    int i_x, i_y;
    for (i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (i_x == 0 || i_y == 0 || i_x == LEVEL_HEIGHT - 1 || i_y == LEVEL_WIDTH - 1) {
                lvl_map[level][i_x][i_y] = T_WALL;
            } /* Around */
            else
                lvl_map[level][i_x][i_y] = T_FLOOR;
        }
    }

    if (level != LEVELS - 1) {
        if (!find_floor_tile(level, &i_x, &i_y, T_FLOOR, 1))
            print_to_log("Could not place stairs down!\n");
        else
            lvl_map[level][i_x][i_y] = T_STAIRS_DOWN;
    }

    if (!find_floor_tile(level, &i_x, &i_y, T_FLOOR, 1))
        print_to_log("Could not place stairs up!\n");
    lvl_map[level][i_x][i_y] = T_STAIRS_UP;
}
