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


/*
 * Creates a room on the map. Will only put walls if there was ground, and floor if the tiles were
 * unwalkable.
 */
void make_room(int level, int top_wall, int bottom_wall, int left_wall, int right_wall)
{
    for (int i_x = top_wall; i_x <= bottom_wall; i_x++) {
        for (int i_y = left_wall; i_y <= right_wall; i_y++) {
            if ((i_x == top_wall || i_x == bottom_wall || i_y == left_wall || i_y == right_wall)
                && lvl_map[level][i_x][i_y] & T_GROUND)
                lvl_map[level][i_x][i_y] = T_WALL;
            else if (lvl_map[level][i_x][i_y] & ~T_WALKABLE)
                lvl_map[level][i_x][i_y] = T_FLOOR;
        }
    }
}


/*
 * Creates a semi-randomly shaped corridor between two points. Will only replace unwalkable tiles.
 */
void make_corridor(int level, int from_x, int from_y, int to_x, int to_y)
{
    // If needed, swap to and from to make sure we go left to right.
    if (from_y > to_y) {
        int tmp = from_x;
        from_x = to_x;
        to_x = tmp;

        tmp = from_y;
        from_y = to_y;
        to_y = tmp;
    }

    int cur_x = from_x;
    int cur_y = from_y;
    int total_y_span = to_y - from_y;
    int total_x_span = abs(to_x - from_x);

    while (cur_y != to_y || cur_x != to_x) {
        // At each iteration, we'll go a certain number of tiles right, then a certain number
        // of tiles up/down.
        int y_span = max(rand_int(total_y_span / 16, total_y_span / 4), 1);
        int y_target = cur_y + min(y_span, to_y - cur_y);

        int x_span = rand_int(total_x_span / 16, total_x_span / 4);
        int x_increment = sign(to_x - cur_x);
        int x_target;

        if (x_increment == 1) {
            x_target = cur_x + min(x_span, to_x - cur_x);
        } else {
            // If we're going down, we'll draw the line from x_target to cur_x instead.
            x_target = cur_x - min(x_span, cur_x - to_x);
            int tmp = cur_x;
            cur_x = x_target;
            x_target = tmp;
        }

        for (int i_y = cur_y; i_y <= y_target; i_y++)
            if (lvl_map[level][cur_x][i_y] & ~T_WALKABLE)
                lvl_map[level][cur_x][i_y] = T_CORRIDOR;

        cur_y = y_target;
        if (cur_y == to_y) {
            // Just finish off the X direction
            if (x_increment == 1)
                x_target = to_x;
            else
                cur_x = to_x;
        }

        for (int i_x = cur_x; i_x <= x_target; i_x++)
            if (lvl_map[level][i_x][cur_y] & ~T_WALKABLE)
                lvl_map[level][i_x][cur_y] = T_CORRIDOR;

        cur_x = x_target;
    }
}


void create_level(int level)
{
    int i_x, i_y;
    for (i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            lvl_map[level][i_x][i_y] = T_GROUND;
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
