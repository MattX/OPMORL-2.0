/*
 *  map.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"


/* Map generator parameters */

const int max_rooms = 7;
const int min_rooms = 5;

// Including walls
const int max_size_x = 8;
const int min_size_x = 4;
const int max_size_y = 15;
const int min_size_y = 3;

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
        int x_target;

        if (y_target == to_y) {
            // Just finish off in the X direction
            x_target = to_x;
        } else if (cur_x < to_x) {
            x_target = cur_x + x_span;
        } else {
            x_target = cur_x - x_span;
        }

        for (int i_y = cur_y; i_y <= y_target; i_y++)
            if (lvl_map[level][cur_x][i_y] & ~T_WALKABLE)
                lvl_map[level][cur_x][i_y] = T_CORRIDOR;

        cur_y = y_target;

        for (int i_x = min(cur_x, x_target); i_x <= max(cur_x, x_target); i_x++)
            if (lvl_map[level][i_x][cur_y] & ~T_WALKABLE)
                lvl_map[level][i_x][cur_y] = T_CORRIDOR;

        cur_x = x_target;
    }
}


void create_level(int level)
{
    int i_x, i_y;

    // Fill everything with ground
    for (i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            lvl_map[level][i_x][i_y] = T_GROUND;
        }
    }

    // Generate some rooms
    int rooms_x[max_rooms], rooms_y[max_rooms]; // for room centers
    int nb_rooms = rand_int(min_rooms, max_rooms);

    for (int i = 0; i < nb_rooms; i++) {
        int room_size_x = rand_int(min_size_x, max_size_x);
        int room_size_y = rand_int(min_size_y, max_size_y);
        int room_x = rand_int(0, LEVEL_HEIGHT - room_size_x - 1);
        int room_y = rand_int(0, LEVEL_WIDTH - room_size_y - 1);

        make_room(level, room_x, room_x + room_size_x, room_y, room_y + room_size_y);

        rooms_x[i] = room_x + room_size_x / 2;
        rooms_y[i] = room_y + room_size_y / 2;
    }

    // Connect rooms
    for (int i = 1; i < nb_rooms; i++) {
        int target_room = rand_int(0, i - 1);
        make_corridor(level, rooms_x[i], rooms_y[i], rooms_x[target_room], rooms_y[target_room]);
    }

    // Add stairs
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
