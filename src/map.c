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
 * Finds a random tile on the specified map level. If can_have_mon is false, the
 * returned tile will not have a monster on it. The tile will be of one of types
 * specified in tile_types.
 * Returns the number of available tiles satisfying the constraints. Stores the coordinates of the
 * empty tile in x and y.
 */
int
find_floor_tile(int level, int *x, int *y, int tile_types, bool can_have_mon)
{
    bool avail[LEVEL_HEIGHT][LEVEL_WIDTH];
    int nb_avail = 0;
    int i_selected;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (lvl_map[level][i_x][i_y] & tile_types) {
                if (!can_have_mon && find_mon_at(i_y, level, i_x) != NULL)
                    avail[i_x][i_y] = false;
                else {
                    avail[i_x][i_y] = true;
                    nb_avail++;
                }
            } else
                avail[i_x][i_y] = false;
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
void make_room(int level, int top_wall, int bottom_wall, int left_wall,
               int right_wall)
{
    for (int i_x = top_wall; i_x <= bottom_wall; i_x++) {
        for (int i_y = left_wall; i_y <= right_wall; i_y++) {
            if ((i_x == top_wall || i_x == bottom_wall || i_y == left_wall ||
                 i_y == right_wall)) {
                if (lvl_map[level][i_x][i_y] == T_GROUND)
                    lvl_map[level][i_x][i_y] = T_WALL;
            } else if (lvl_map[level][i_x][i_y] & ~T_WALKABLE)
                lvl_map[level][i_x][i_y] = T_FLOOR;
        }
    }
}


/*
 * Creates a semi-randomly shaped corridor between two points. Will only replace
 * unwalkable tiles.
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


        // TODO: add support for door generation
        for (int i_y = cur_y; i_y <= y_target; i_y++) {
            if (lvl_map[level][cur_x][i_y] & ~T_WALKABLE)
                lvl_map[level][cur_x][i_y] = T_CORRIDOR;

            if (cur_x != 0 && lvl_map[level][cur_x - 1][i_y] == T_GROUND)
                lvl_map[level][cur_x - 1][i_y] = T_WALL;
            if (cur_x < LEVEL_HEIGHT - 1 &&
                lvl_map[level][cur_x + 1][i_y] == T_GROUND)
                lvl_map[level][cur_x + 1][i_y] = T_WALL;
        }

        cur_y = y_target;

        for (int i_x = min(cur_x, x_target);
             i_x <= max(cur_x, x_target); i_x++) {
            if (lvl_map[level][i_x][cur_y] & ~T_WALKABLE)
                lvl_map[level][i_x][cur_y] = T_CORRIDOR;

            if (cur_y > 0 && lvl_map[level][i_x][cur_y - 1] == T_GROUND)
                lvl_map[level][i_x][cur_y - 1] = T_WALL;
            if (cur_y < LEVEL_WIDTH - 1 &&
                lvl_map[level][i_x][cur_y + 1] == T_GROUND)
                lvl_map[level][i_x][cur_y + 1] = T_WALL;
        }

        cur_x = x_target;
    }
}


/*
 * can walk: Checks if there is a walkable path from (from_x, from_y) to
 * (to_x, to_y). If dir_x and dir_y are not NULL, they will be set to the
 * coordinates of the first tile adjacent to (from_x, from_y) in the path
 * from (from_x, from_y) to (to_x, to_y).
 */
int can_walk(int level, int from_x, int from_y, int to_x, int to_y)
{
    bool checked[LEVEL_HEIGHT][LEVEL_WIDTH];
    int stack_x[LEVEL_HEIGHT * LEVEL_WIDTH];
    int stack_y[LEVEL_HEIGHT * LEVEL_WIDTH];
    int stack_pointer = 0;

    memset(checked, false, LEVEL_HEIGHT * LEVEL_WIDTH);

    stack_x[0] = from_x;
    stack_y[0] = from_y;

    while (stack_pointer >= 0) {
        int cur_x = stack_x[stack_pointer];
        int cur_y = stack_y[stack_pointer];
        stack_pointer--;
        checked[cur_x][cur_y] = true;

        if (cur_x == to_x && cur_y == to_y)
            return true;

        for (int x = cur_x - 1; x <= cur_x + 1; x++) {
            if (x < 0 || x >= LEVEL_HEIGHT)
                continue;
            for (int y = cur_y - 1; y <= cur_y + 1; y++) {
                if (y < 0 || y >= LEVEL_WIDTH)
                    continue;
                if (lvl_map[level][x][y] & T_WALKABLE && !checked[x][y]) {
                    stack_pointer++;
                    stack_x[stack_pointer] = x;
                    stack_y[stack_pointer] = y;
                }
            }
        }

    }

    return false;
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
    int rooms_x[max_rooms], rooms_y[max_rooms];
    bool connected[max_rooms];
    int nb_rooms = rand_int(min_rooms, max_rooms);

    for (int i = 0; i < nb_rooms; i++) {
        int room_size_x = rand_int(min_size_x, max_size_x);
        int room_size_y = rand_int(min_size_y, max_size_y);
        int room_x = rand_int(0, LEVEL_HEIGHT - room_size_x - 1);
        int room_y = rand_int(0, LEVEL_WIDTH - room_size_y - 1);

        make_room(level, room_x, room_x + room_size_x, room_y,
                  room_y + room_size_y);

        rooms_x[i] = room_x + room_size_x / 2;
        rooms_y[i] = room_y + room_size_y / 2;
        connected[i] = false;
    }

    // Check room connections
    connected[0] = true;
    for (int i = 1; i < nb_rooms; i++) {
        if (can_walk(level, rooms_x[0], rooms_y[0], rooms_x[i], rooms_y[i]))
            connected[i] = true;
    }

    // Connect rooms
    for (int i = 1; i < nb_rooms; i++) {
        if (!connected[i]) {
            int target_room;
            do {
                target_room = rand_int(0, nb_rooms - 1);
            } while (!connected[target_room]);
            make_corridor(level, rooms_x[i], rooms_y[i], rooms_x[target_room],
                          rooms_y[target_room]);
            connected[i] = true;
        }
    }

    // Add stairs
    if (level != LEVELS - 1) {
        if (!find_floor_tile(level, &i_x, &i_y, T_FLOOR, 1))
            print_to_log("Could not place stairs down on level %d!\n", level);
        else
            lvl_map[level][i_x][i_y] = T_STAIRS_DOWN;
    }

    if (!find_floor_tile(level, &i_x, &i_y, T_FLOOR, 1))
        print_to_log("Could not place stairs up on level %d!\n", level);
    lvl_map[level][i_x][i_y] = T_STAIRS_UP;

    // Add objects
    add_level_objects(level);

    // Set visibility map
    for (i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            visibility_map[level][i_x][i_y] = TS_UNDISCOVERED;
}

/*
 * Bresenham-like raytracer
 */
void set_visible(int level, int from_x, int from_y, int to_x, int to_y)
{
    int dx = abs(to_x - from_x);
    int sx = sign(to_x - from_x);
    int dy = abs(to_y - from_y);
    int sy = sign(to_y - from_y);
    int error = (dx > dy ? dx : -dy) / 2;
    int old_error;

    int cur_x = from_x, cur_y = from_y;
    bool blocked = false;

    while (1) {
        if (blocked && visibility_map[rodney.dlvl][cur_x][cur_y] == TS_SEEN)
            visibility_map[rodney.dlvl][cur_x][cur_y] = TS_UNSEEN;
        if (!blocked)
            visibility_map[rodney.dlvl][cur_x][cur_y] = TS_SEEN;

        if (lvl_map[level][cur_x][cur_y] & ~T_WALKABLE)
            blocked = true;

        if (cur_x == to_x && cur_y == to_y)
            break;

        old_error = error;
        if (old_error > -dx) {
            error -= dy;
            cur_x += sx;
        }
        if (old_error < dy) {
            error += dx;
            cur_y += sy;
        }
    }
}

void recompute_visibility()
{
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            if (i_x == 0 || i_x == LEVEL_HEIGHT - 1 || i_y == 0 ||
                i_y == LEVEL_WIDTH - 1)
                set_visible(rodney.dlvl, rodney.posx, rodney.posy, i_x, i_y);
}


struct s_coord
{
    int x;
    int y;
};

bool
dijkstra(int level, int from_x, int from_y, int to_x, int to_y, int *next_x,
         int *next_y, bool can_have_monst)
{
    struct s_coord tiles[2][LEVEL_HEIGHT * LEVEL_WIDTH];
    struct s_coord prev[LEVEL_HEIGHT][LEVEL_WIDTH];
    bool visited[LEVEL_HEIGHT][LEVEL_WIDTH];

    int nb_tiles[2] = {1, 0};
    int rotation = 0, new_rotation = 1;

    if (from_x == to_x && from_y == to_y)
        return false;

    tiles[rotation][0].x = from_x;
    tiles[rotation][0].y = from_y;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            visited[i_x][i_y] = false;

    while (nb_tiles[rotation] > 0) {
        for (int i_tile = 0; i_tile < nb_tiles[rotation]; i_tile++) {
            int cur_x = tiles[rotation][i_tile].x;
            int cur_y = tiles[rotation][i_tile].y;

            for (int i_x = cur_x - 1; i_x <= cur_x + 1; i_x++) {
                if (i_x < 0 || i_x > LEVEL_HEIGHT)
                    continue;

                for (int i_y = cur_y - 1; i_y <= cur_y + 1; i_y++) {
                    if (i_y < 0 || i_y > LEVEL_WIDTH)
                        continue;

                    if (!visited[i_x][i_y] &&
                        lvl_map[level][i_x][i_y] & T_WALKABLE &&
                        (can_have_monst ||
                         find_mon_at(level, i_x, i_y) == NULL)) {
                        tiles[new_rotation][nb_tiles[new_rotation]].x = i_x;
                        tiles[new_rotation][nb_tiles[new_rotation]].y = i_y;
                        prev[i_x][i_y] = tiles[rotation][i_tile];
                        visited[i_x][i_y] = true;
                        nb_tiles[new_rotation]++;
                    }

                    if (i_x == to_x && i_y == to_y) {
                        // Backtrack to find tile to go to
                        struct s_coord coming_from;
                        struct s_coord cur = {.x = i_x, .y = i_y};

                        while (cur.x != from_x || cur.y != from_y) {
                            coming_from = cur;
                            cur = prev[cur.x][cur.y];
                        }
                        *next_x = coming_from.x;
                        *next_y = coming_from.y;

                        return true;
                    }
                }
            }
        }

        nb_tiles[rotation] = 0;
        rotation = !rotation;
        new_rotation = !new_rotation;
    }

    return false;
}