/*
 *  map.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include <unistd.h>
#include <math.h>
#include "opmorl.h"

/**
 * Map tile definitions
 * MUST BE in the same order as tile definitions in opmorl.h
 */
struct s_tile_type tile_types[NB_TILE_TYPES] =
        {
                /* walkable, LOS, sym, color */
                {false, false, '#', CLR_DEFAULT},   // wall
                {true,  true,  '.', CLR_DEFAULT},   // corridor
                {true,  true,  'W', CLR_DEFAULT},   // open door (special sym)
                {false, false, '+', CLR_DEFAULT},   // closed door
                {true,  true,  '.', CLR_DEFAULT},   // stone floor
                {true,  true,  '<', CLR_DEFAULT},   // stairs down
                {true,  true,  '>', CLR_DEFAULT},   // stairs up
                {false, false, ' ', CLR_DEFAULT},   // ground
                {false, true,  '~', CLR_DEFAULT},   // collapsed
                {true,  true,  '^', CLR_DEFAULT},   // locked trapdoor
                {false, true,  '-', CLR_DEFAULT},   // open trapdoor
                {true,  true,  '/', CLR_YELLOW},    // lever
                {false, false, 'W', CLR_YELLOW},    // pipe (special sym)
                {false, false, 'o', CLR_YELLOW},    // pipe exhaust
                {true,  true,  ';', CLR_GREEN},     // grass
                {true,  true,  ';', CLR_MAGENTA},   // fungus
                {false, false, 'T', CLR_GREEN},     // tree
                {true,  true,  '}', CLR_BLUE},      // fountain
                {true,  true,  '.', CLR_BLUE},      // portcullis up
                {false, true,  '#', CLR_BLUE},      // portcullis down
                {true,  false, '*', CLR_DEFAULT},   // rubble
        };


/** Size of a chasm */
const int chasm_width = 40;
const int chasm_height = 15;

/** Size of a patch area */
const int patch_width = 7;
const int patch_height = 4;

/** Percent chance that a door will be generated open */
const int open_door_chance = 15;
/** Percent chance that a portcullis will be generated, if possible */
const int portcullis_chance = 10;


/**
 * Checks if the coordinates are valid (ie in the map)
 */
inline bool valid_coordinates(Coord pos)
{
    return (pos.x >= 0 && pos.x < LEVEL_HEIGHT && pos.y >= 0 &&
            pos.y < LEVEL_WIDTH);
}

/**
 * Checks if the coordinates point to an outer wall
 */
static bool is_outer_wall(Coord pos)
{
    return (pos.x == 0 || pos.y == 0 || pos.x == LEVEL_HEIGHT - 1 ||
            pos.y == LEVEL_WIDTH - 1);
}

/**
 * Helper function: returns the coordinates of a neighboring cell.
 * @param around The point around which to return a neighbor
 * @param n The neighbor index:
 *    4  0  5
 *    1  .  2
 *    6  3  7
 * @warning May return invalid (out-of-map) coordinates.
 */
Coord get_neighbor(Coord around, int n)
{
    Coord result = around;

    if (n == 4 || n == 0 || n == 5)
        result.x--;
    if (n == 6 || n == 3 || n == 7)
        result.x++;
    if (n == 4 || n == 1 || n == 6)
        result.y--;
    if (n == 5 || n == 2 || n == 7)
        result.y++;

    return result;
}


/**
 * Returns the orientation of a tile type at a given coordinate. This is made
 * to recognize the following situations (assuming type == wall)
 *
 *     ...          .#.        .#.  .#.
 *     #X#          .X.        .X.  .X#
 *     ...          .#.        ...  .#.
 *  Horizontal    Vertical       None
 *
 * @param dlvl
 * @param pos
 * @param type
 * @return 1 for horizontal, -1 for vertical, 0 for none
 */
int get_orientation(int dlvl, Coord pos, TileType type)
{
    // There is no orientation at the edge of the map
    if (is_outer_wall(pos))
        return 0;

    bool up = maps[dlvl][pos.x - 1][pos.y] == type;
    bool down = maps[dlvl][pos.x + 1][pos.y] == type;
    bool left = maps[dlvl][pos.x][pos.y - 1] == type;
    bool right = maps[dlvl][pos.x][pos.y + 1] == type;

    if (up && down && !left && !right)
        return -1;
    else if (!up && !down && left && right)
        return 1;
    else
        return 0;
}


/**
 * Registers a connection between a lever and a target
 * @param dlvl, pos The position of the lever
 * @param target_dvl, target_pos The position of the target
 */
static void register_lever(int dlvl, Coord pos, int target_dlvl, Coord target)
{
    LeverConnection *connection = malloc(sizeof(LeverConnection));
    connection->dlvl = dlvl;
    connection->pos = pos;
    connection->target_dlvl = target_dlvl;
    connection->target_pos = target;

    add_to_linked_list(lever_connections, connection);
}


/**
 * Checks if a given tile fits the following criteria:
 *  - If tile_type is outside the range [0, NB_TILE_TYPES-1], the tile must
 *    be walkable
 *  - Else, the tile must be of the given type
 *  - If can_have_mon is false, checks that there are no monster on the tile.
 *
 * @param dlvl, pos The coordinates of the cell to check
 * @param can_have_mon Whether the tile can have a monster on it
 * @param tile_type A tile type to look for
 */
static bool fits_criteria(int dlvl, Coord pos, bool can_have_mon, int tile_type)
{
    bool looking_for_specific_type =
            tile_type >= 0 && tile_type < NB_TILE_TYPES;

    if (((!looking_for_specific_type && IS_WALKABLE(maps[dlvl][pos.x][pos.y]))
         ||
         (looking_for_specific_type && maps[dlvl][pos.x][pos.y] == tile_type))
        &&
        (!can_have_mon || (find_mon_at(dlvl, pos) == NULL &&
                           (dlvl != rodney.dlvl || pos.x != rodney.pos.x ||
                            pos.y != rodney.pos.y)))) {
        return true;
    }
    return false;
}


/**
 * Finds a random true tile in a mask.
 * @param coords Pointer where the location of a tile will be stored.
 * @param mask A LEVEL_HEIGHT*LEVEL_WIDTH array of boolean values.
 * @return The number of true tiles. Can be 0.
 */
static int find_tile_mask(Coord *coords, bool *mask)
{
    int nb_avail = 0;
    int i_selected;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (mask[i_x * LEVEL_WIDTH + i_y])
                nb_avail++;
        }
    }

    if (nb_avail == 0)
        return 0;

    i_selected = rand_int(0, nb_avail - 1);
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (mask[i_x * LEVEL_WIDTH + i_y]) {
                if (i_selected == 0) {
                    coords->x = i_x;
                    coords->y = i_y;
                    return nb_avail;
                }
                i_selected--;
            }
        }
    }

    // Should not reach here
    print_to_log("Something's gone wrong in find_tile_mask().\n");
    return 0;
}


/**
 * Returns a random walkable tile on the specified map level, fitting given
 * criteria.
 * @param coords Pointer where the location of a tile will be stored.
 * @param dlvl The dungeon level on which to perform the search
 * @param can_have_mon If false, the tile will not have a monster on it.
 * @param tile_type If set in the range [0, NB_TILE_TYPES-1], will only look
 * for tiles of that type. The tile type doesn't have to be walkable. Otherwise,
 * will return a random *walkable* tile.
 * @returns The number of available tiles satisfying the constraints. Can be 0.
 */
int
find_tile(Coord *coords, int dlvl, bool can_have_mon, int tile_type)
{
    bool *mask = malloc(LEVEL_HEIGHT * LEVEL_WIDTH * sizeof(bool));

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (fits_criteria(dlvl, (Coord) {i_x, i_y}, can_have_mon,
                              tile_type)) {
                mask[i_x * LEVEL_WIDTH + i_y] = true;
            } else {
                mask[i_x * LEVEL_WIDTH + i_y] = false;
            }
        }
    }

    int nb_avail = find_tile_mask(coords, mask);

    free(mask);

    return nb_avail;
}


/**
 * Finds the closest (walkable) tile to a specified tile.
 * @param coords A pointer to a Coord struct where the coordinates will be
 * stored
 * @param dlvl The depth level to search on
 * @param near The coordinates around which we look
 * @param can_have_mon Whether the tile can have a monster on it
 * @param tile_type If in [0, NB_TILE_TYPES-1], specifies a tile type
 * @return Whether a tile was found.
 */
bool find_closest(Coord *coords, int dlvl, Coord near, bool can_have_mon,
                  int tile_type)
{
    for (int radius = 0; radius < LEVEL_WIDTH; radius++) {
        for (int i_x = near.x - radius; i_x <= near.x + radius; i_x++) {
            for (int i_y = near.y - radius; i_y <= near.y + radius; i_y++) {
                if (i_x != near.x - radius && i_x != near.x + radius &&
                    i_y != near.y - radius && i_y != near.y + radius)
                    continue;
                if (!valid_coordinates((Coord) {i_x, i_y}))
                    continue;

                if (fits_criteria(dlvl, (Coord) {i_x, i_y}, can_have_mon,
                                  tile_type)) {
                    coords->x = i_x;
                    coords->y = i_y;
                    return true;
                }
            }
        }
    }

    return false;
}


/**
 * Creates a semi-randomly shaped corridor between two points. Will only replace
 * unwalkable tiles.
 * @param level The level in which to create the corridor
 * @param from, to Endpoint coordinates
 */
static void make_path(TileType (*level)[LEVEL_WIDTH], Coord from, Coord to)
{
    // If needed, swap to and from to make sure we go left to right.
    if (from.y > to.y) {
        int tmp = from.x;
        from.x = to.x;
        to.x = tmp;

        tmp = from.y;
        from.y = to.y;
        to.y = tmp;
    }

    int cur_x = from.x;
    int cur_y = from.y;
    int total_y_span = to.y - from.y;
    int total_x_span = abs(to.x - from.x);

    while (cur_y != to.y || cur_x != to.x) {
        // At each iteration, we'll go a certain number of tiles right, then a
        // certain number of tiles up/down.
        int y_span = max(rand_int(total_y_span / 16, total_y_span / 4), 1);
        int y_target = cur_y + min(y_span, to.y - cur_y);

        int x_span = rand_int(total_x_span / 16, total_x_span / 4);
        int x_target;

        if (y_target == to.y) {
            // Just finish off in the X direction
            x_target = to.x;
        } else if (cur_x < to.x) {
            x_target = cur_x + x_span;
        } else {
            x_target = cur_x - x_span;
        }

        for (int i_y = cur_y; i_y <= y_target; i_y++) {
            if (!IS_WALKABLE(level[cur_x][i_y]))
                level[cur_x][i_y] = T_FLOOR;
        }

        cur_y = y_target;

        for (int i_x = min(cur_x, x_target);
             i_x <= max(cur_x, x_target); i_x++) {
            if (!IS_WALKABLE(level[i_x][cur_y]))
                level[i_x][cur_y] = T_FLOOR;
        }

        cur_x = x_target;
    }
}


/**
 * Checks if there is a walkable path from a point to another in an abstract
 * level representation (mask).
 *
 * @param mask The mask on which to perform the check. `true` cells are
 * considered walkable, others not.
 * @param width The width of the mask.
 * @param from Coordinates of the first point.
 * @param to Coordinates of the second point.
 * @return Whether there is a path from the first point to the second.
 */
static bool can_walk_mask(bool *mask, int width, Coord from, Coord to)
{
    bool checked[LEVEL_HEIGHT][LEVEL_WIDTH];
    Coord stack[LEVEL_HEIGHT * LEVEL_WIDTH];
    int stack_pointer = 0;

    memset(checked, false, LEVEL_HEIGHT * LEVEL_WIDTH);

    if (!mask[from.x * width + from.y]) {
        return false;
    }

    stack[0] = from;

    while (stack_pointer >= 0) {
        Coord cur_root = stack[stack_pointer];
        stack_pointer--;

        if (cur_root.x == to.x && cur_root.y == to.y)
            return true;

        for (int i_neighbor = 0; i_neighbor < 8; i_neighbor++) {
            Coord cur = get_neighbor(cur_root, i_neighbor);
            if (!valid_coordinates(cur))
                continue;

            if (mask[cur.x * width + cur.y] && !checked[cur.x][cur.y]) {
                stack_pointer++;
                stack[stack_pointer] = cur;
                checked[cur.x][cur.y] = true;
            }
        }
    }

    return false;
}


/**
 * Checks if there is a walkable path from a point to another. A path is
 * considered walkable if it only contains walkable tiles or closed doors.
 *
 * @param dlvl The level at which to perform the check
 * @param from Coordinates of the first point.
 * @param to Coordinates of the second point.
 * @return Whether there is a path from the first point to the second.
 */
static bool can_walk(int dlvl, Coord from, Coord to)
{
    bool *mask = calloc((size_t) LEVEL_HEIGHT * LEVEL_WIDTH, sizeof(bool));

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (IS_WALKABLE(maps[dlvl][i_x][i_y]) ||
                maps[dlvl][i_x][i_y] == T_DOOR_CLOSED) {
                mask[i_x * LEVEL_WIDTH + i_y] = true;
            }
        }
    }

    bool can_walk = can_walk_mask(mask, LEVEL_WIDTH, from, to);
    free(mask);
    return can_walk;
}


/**
 * Checks if it is possible to walk between two points in a level where a
 * blob has been added. Used to check that the blob will not prevent essential
 * movement.
 *
 * @param blob A blob representation. true cells represent areas made unwalkable
 * by the blob.
 * @param height, width Blob size
 * @param blob_pos Upper left coordinates of the blob
 * @param dlvl Depth level
 * @param from Start point
 * @param to End point
 * @return Whether it is possible to walk between the start and end points.
 */
static bool can_walk_blob(bool *blob, int height, int width, Coord blob_pos,
                          int dlvl, Coord from, Coord to)
{
    if (blob_pos.x < 0 || blob_pos.y < 0 ||
        blob_pos.x + height >= LEVEL_HEIGHT ||
        blob_pos.y + width >= LEVEL_WIDTH) {
        print_to_log("WARN: tried to check blob outside of level.\n");
        return false;
    }

    bool *mask = calloc((size_t) LEVEL_HEIGHT * LEVEL_WIDTH, sizeof(bool));

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if ((IS_WALKABLE(maps[dlvl][i_x][i_y]) ||
                 maps[dlvl][i_x][i_y] == T_DOOR_CLOSED)) {
                mask[i_x * LEVEL_WIDTH + i_y] = true;
            }
        }
    }

    for (int i_x = 0; i_x < height; i_x++) {
        for (int i_y = 0; i_y < width; i_y++) {
            if (blob[i_x * width + i_y])
                mask[(i_x + blob_pos.x) * LEVEL_WIDTH +
                     (i_y + blob_pos.y)] = false;
        }
    }

    bool can_walk = can_walk_mask(mask, LEVEL_WIDTH, from, to);
    free(mask);
    return can_walk;
}


/**
 * Checks whether a tile is visible from another tile on a given level.
 * Walkable tiles are considered see-through, other are opaque. The first
 * opaque tile on the way is considered visible. The algorithm should be
 * invertible, that is, if point A is visible from point B, then B is
 * visible from A.
 *
 * @param dlvl The level on which to perform the calculation
 * @param from The coordinates of the source point
 * @param to The coordinates of the destination point
 * @param block Pointers that will be updated with the coordinates
 * of the blocking tile (if not set to NULL).
 * @param monsters_block If set to true, monsters will count as unwalkable
 * (opaque) tiles. This is useful for ranged weapons calculation.
 * @return whether the tile is visible
 */
bool
is_visible(int dlvl, Coord from, Coord to, Coord *block, bool monsters_block)
{
    int dx = abs(to.x - from.x);
    int sx = sign(to.x - from.x);
    int dy = abs(to.y - from.y);
    int sy = sign(to.y - from.y);
    int error = (dx > dy ? dx : -dy) / 2;
    int old_error;

    Coord cur = from;
    bool blocked = false;

    while (1) {
        if (blocked)
            return false;

        if (cur.x == to.x && cur.y == to.y)
            return true;

        if (!IS_TRANSPARENT(maps[dlvl][cur.x][cur.y]) ||
            (monsters_block && find_mon_at(dlvl, cur))) {
            blocked = true;

            if (block != NULL) {
                *block = cur;
            }
        }

        old_error = error;
        if (old_error > -dx) {
            error -= dy;
            cur.x += sx;
        }
        if (old_error < dy) {
            error += dx;
            cur.y += sy;
        }
    }
}


/**
 * Computes the visibility map for the current level, and stores it in the
 * global visibility map array.
 */
void recompute_visibility()
{
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            bool visible = is_visible(rodney.dlvl, rodney.pos,
                                      (Coord) {i_x, i_y}, NULL, false);

            if (visible || god_mode)
                visibility_map[rodney.dlvl][i_x][i_y] = TS_SEEN;

            else if (visibility_map[rodney.dlvl][i_x][i_y] != TS_UNDISCOVERED)
                visibility_map[rodney.dlvl][i_x][i_y] = TS_UNSEEN;

            else
                visibility_map[rodney.dlvl][i_x][i_y] = TS_UNDISCOVERED;
        }
}


/**
 * Runs Dijkstra's algorithm between two points.
 *
 * Djikstra's is simplified by the fact that there is a constant distance
 * between all tiles, so it's really just a BFS. The `prev` array stores
 * the coordinates of the parent tile for every tile, which allows us to
 * backtrack and optionally return the first step on the shortest path.
 *
 * @param dlvl The level on which to run the algorithm
 * @param from The source point coordinates
 * @param to The destination point coordinates
 * @param next Pointer in which the next position in the shortest path will be
 * stored if true is returned. Can be set to NULL.
 * @param can_have_monst If set to false, only paths containing no monsters
 * will be considered.
 * @return Whether there is a path between the source and destionation points.
 */
bool dijkstra(int dlvl, Coord from, Coord to, Coord *next,
              bool can_have_monst)
{
    Coord tiles[2][LEVEL_HEIGHT * LEVEL_WIDTH];
    Coord prev[LEVEL_HEIGHT][LEVEL_WIDTH];
    bool visited[LEVEL_HEIGHT][LEVEL_WIDTH];

    int nb_tiles[2] = {1, 0};
    int rotation = 0, new_rotation = 1;

    if (from.x == to.x && from.y == to.y)
        return false;

    tiles[rotation][0] = from;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            visited[i_x][i_y] = false;

    while (nb_tiles[rotation] > 0) {
        for (int i_tile = 0; i_tile < nb_tiles[rotation]; i_tile++) {
            Coord parent = tiles[rotation][i_tile];

            for (int i_neighbor = 0; i_neighbor < 8; i_neighbor++) {
                Coord cur = get_neighbor(parent, i_neighbor);
                if (!valid_coordinates(cur))
                    continue;

                if (!visited[cur.x][cur.y] &&
                    IS_WALKABLE(maps[dlvl][cur.x][cur.y]) &&
                    (can_have_monst || find_mon_at(dlvl, cur) == NULL)) {
                    tiles[new_rotation][nb_tiles[new_rotation]] = cur;
                    prev[cur.x][cur.y] = parent;
                    visited[cur.x][cur.y] = true;
                    nb_tiles[new_rotation]++;
                }

                if (cur.x == to.x && cur.y == to.y) {
                    if (!visited[cur.x][cur.y]) {
                        return false;
                    }

                    // Backtrack to find tile to go to
                    Coord backtrack_cur = cur;
                    Coord coming_from = parent;

                    while (backtrack_cur.x != from.x ||
                           backtrack_cur.y != from.y) {
                        coming_from = backtrack_cur;
                        backtrack_cur =
                                prev[backtrack_cur.x][backtrack_cur.y];
                    }

                    *next = coming_from;

                    return true;
                }
            }
        }

        nb_tiles[rotation] = 0;
        rotation = !rotation;
        new_rotation = !new_rotation;
    }

    return false;
}

/**
 * Helper function to place a level
 * @param type The type of the level to place
 * @param position The number of normal levels above
 */
static void place_level(enum e_dungeon_level_type type, int position)
{
    int i_dlvl;

    for (i_dlvl = 0; i_dlvl < DLVL_MAX && position != 0; i_dlvl++) {
        if (dlvl_types[i_dlvl] == DLVL_NORMAL)
            position--;
    }
    dlvl_types[i_dlvl] = type;
}


/**
 * Picks a layout for the dungeon levels.
 * - Mark the last level as DLVL_LAST
 * - Mark a random level between 1/3 and 2/3 as the administrator's level
 * - Mark a random level between the administrator's level and the bottom
 *   as the archmage's level
 * - Either (50/50) put a bank and a market or barracks somewhere.
 * - Flood the last 2-4 levels
 * - Try to replace some floors with maintenance levels
 */
void layout_dungeon()
{
    int remaining_levels = DLVL_MAX;

    for (int i_dlvl = 0; i_dlvl < DLVL_MAX; i_dlvl++) {
        dlvl_types[i_dlvl] = DLVL_NORMAL;
        dlvl_flags[i_dlvl] = 0;
    }

    dlvl_types[DLVL_MAX - 1] = DLVL_LAST;
    remaining_levels--;

    int administrator = rand_int(DLVL_MAX / 3, 2 * DLVL_MAX / 3 - 1);
    dlvl_types[administrator] = DLVL_ADMINISTRATOR;
    remaining_levels--;

    int archmage = rand_int(administrator + 1, DLVL_MAX - 2);
    dlvl_types[archmage] = DLVL_ARCHMAGE;
    remaining_levels--;

    if (rand_int(0, 1) == 0) {
        // Bank and market
        int market = rand_int(1, remaining_levels - 2);
        place_level(DLVL_MARKET, market);
        remaining_levels--;

        int bank = rand_int(market + 1, remaining_levels - 1);
        place_level(DLVL_BANK, bank);
    } else {
        // Barracks
        int barracks = rand_int(1, remaining_levels - 1);
        place_level(DLVL_BARRACKS, barracks);
    }

    for (int i = 0; i < 4; i++) {
        int candidate = rand_int(1, DLVL_MAX - 1);
        if (dlvl_types[candidate] == DLVL_NORMAL)
            dlvl_types[candidate] = DLVL_MAINTENANCE;
    }

    int nb_flooded = rand_int(2, 4);
    for (int i = DLVL_MAX - 1; i > DLVL_MAX - 1 - nb_flooded; i--) {
        dlvl_flags[i] |= DFLAGS_FLOODED;
    }
}


/**
 * Helper function to count the number of neighbors of a cell that are set to
 * true in a boolean array.
 * @param array The array in which to perform the check
 * @param height, width The dimensions of the array
 * @param x, y The coordinates of the cell to check
 * @return The number of neighbors of that cell that are set to true.
 */
static int count_neighbors(bool *array, int height, int width, Coord cell)
{
    int count = 0;

    for (int i_neighbor = 0; i_neighbor < 8; i_neighbor++) {
        Coord neighbor = get_neighbor(cell, i_neighbor);

        if (neighbor.x < 0 || neighbor.x >= height || neighbor.y < 0 ||
            neighbor.y >= width)
            continue;

        if (array[neighbor.x * width + neighbor.y])
            count++;
    }

    return count;
}

/**
 * Generate a random blob, approximately of given size.
 *
 * This is done by selecting 45% of the cells at random, and then applying
 * 5 successive rounds of smoothing to the resulting shape.
 *
 * The method is from here:
 * https://www.rockpapershotgun.com/2015/07/28/how-do-roguelikes-generate-levels/
 *
 * @param array An allocated array that will be filled in with the shape.
 * @param height, width The dimensions of the array (the shape will be about
 * that size)
 */
static void make_blob(bool *array, int height, int width)
{
    bool steps[2][height][width];

    int parity = 0;

    for (int i_x = 0; i_x < height; i_x++) {
        for (int i_y = 0; i_y < width; i_y++) {
            if (rand_int(1, 100) <= 45)
                steps[0][i_x][i_y] = false;
            else
                steps[0][i_x][i_y] = true;
        }
    }

    for (int iteration = 0; iteration < 5; iteration++) {
        parity = !parity;
        for (int i_x = 0; i_x < height; i_x++) {
            for (int i_y = 0; i_y < width; i_y++) {
                int true_neighbors = count_neighbors((bool *) steps[!parity],
                                                     height, width,
                                                     (Coord) {i_x, i_y});

                if (true_neighbors < 4)
                    steps[parity][i_x][i_y] = false;
                else if (true_neighbors >= 6)
                    steps[parity][i_x][i_y] = true;
                else
                    steps[parity][i_x][i_y] = steps[!parity][i_x][i_y];
            }
        }
    }

    for (int i_x = 0; i_x < height; i_x++) {
        for (int i_y = 0; i_y < width; i_y++) {
            array[i_x * width + i_y] = steps[parity][i_x][i_y];
        }
    }
}


/**
 * Adds a blob of some type of tile to the level
 * @param dlvl Depth level
 * @param blob Blob array
 * @param height, width Blob size
 * @param blob_pos Position of upper left corner of blob
 * @param tile Tile to replace blob with
 */
static void blit_blob(int dlvl, bool *blob, int height, int width,
                      Coord blob_pos, TileType tile)
{
    if (blob_pos.x < 0 || blob_pos.y < 0 ||
        blob_pos.x + height >= LEVEL_HEIGHT ||
        blob_pos.y + width >= LEVEL_WIDTH) {
        print_to_log("WARN: tried to blit blob outside of level.\n");
        return;
    }

    for (int i_x = 0; i_x < height; i_x++) {
        for (int i_y = 0; i_y < width; i_y++) {
            if (blob[i_x * width + i_y])
                maps[dlvl][i_x + blob_pos.x][i_y + blob_pos.y] = tile;
        }
    }
}


/** Grid loading features **/

enum e_grid_tile
{
    GT_UNREMOVABLE_WALL, GT_WALL, GT_SEPARATOR, GT_FLOOR
};

enum e_grid_tile grid[LEVEL_HEIGHT][LEVEL_WIDTH];

/**
 * Loads the level grid
 * @return whether the call succeeded.
 */
bool load_grid()
{
    FILE *grid_file = fopen(GRID_FILE, "r");

    if (grid_file == NULL) {
        print_to_log("Could not open grid file %s\n", GRID_FILE);
        fclose(grid_file);
        return false;
    }

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            char c;

            // SCANF DARK MAGIC HAHA
            if (!fscanf(grid_file, " %c ", &c)) {
                print_to_log(
                        "Ran out of characters while loading the grid at (%d,%d).\n",
                        i_x, i_y);
                fclose(grid_file);
                return false;
            }

            switch (c) {
            case '#':
                grid[i_x][i_y] = GT_UNREMOVABLE_WALL;
                break;
            case 'x':
                grid[i_x][i_y] = GT_WALL;
                break;
            case '+':
                grid[i_x][i_y] = GT_SEPARATOR;
                break;
            case '.':
                grid[i_x][i_y] = GT_FLOOR;
                break;
            default:
                print_to_log("**Unknown char '%c' in grid file.\n", c);
                return false;
            }
        }
    }

    fclose(grid_file);

    return true;
}


/**
 * Replace a continuous set of grid tiles with another tile type on a given
 * level. Only side (not diagonal) connections are considered.
 * @param level A level map in which to perform the replacement
 * @param from_x, from_y The coordinates from which to start the replacement
 * @param to_type The new tile type
 */
static void replace_tiles_from(TileType (*level)[LEVEL_WIDTH], Coord from,
                               int source_type, TileType to_type)
{
    if (grid[from.x][from.y] != source_type || level[from.x][from.y] == to_type)
        return;

    level[from.x][from.y] = to_type;

    for (int i_neighbor = 0; i_neighbor < 4; i_neighbor++) {
        Coord neighbor = get_neighbor(from, i_neighbor);
        if (!valid_coordinates(neighbor))
            continue;
        replace_tiles_from(level, neighbor, source_type, to_type);
    }
}


/**
 * Returns true if the tile is surrounded by walkable tiles in all four
 * directions, false otherwise. If the tile is on the edge of the map, the
 * function will return false.
 * @param level A level map in which to perform this check
 * @param from_x, from_y The coordinates to check
 * @return Whether the tile is surrounded by walkable tiles
 */
static bool surrounded_by_walkable(TileType (*level)[LEVEL_WIDTH], Coord pos)
{
    if (is_outer_wall(pos))
        return false;

    if (IS_WALKABLE(level[pos.x - 1][pos.y]) &&
        IS_WALKABLE(level[pos.x + 1][pos.y]) &&
        IS_WALKABLE(level[pos.x][pos.y - 1]) &&
        IS_WALKABLE(level[pos.x][pos.y + 1]))
        return true;
    else
        return false;
}


/**
 * Checks if this is a reasonable position to place a door.
 * @param level Level on which to check
 * @param x, y Coordinates of the place to check
 * @return Whether a door can be placed at that position
 */
static bool can_place_door(TileType (*level)[LEVEL_WIDTH], Coord pos)
{
    // We shouldn't be on an edge of the map
    if (is_outer_wall(pos))
        return false;

    // We should be overwriting a boring block
    TileType tile = level[pos.x][pos.y];
    if (tile != T_FLOOR && tile != T_WALL && tile != T_GROUND &&
        tile != T_CORRIDOR)
        return false;

    // We shouldn't be touching another door
    for (int i_x = pos.x - 1; i_x <= pos.x + 1; i_x++)
        for (int i_y = pos.y - 1; i_y <= pos.y + 1; i_y++)
            if (level[i_x][i_y] == T_DOOR_OPEN ||
                level[i_x][i_y] == T_DOOR_CLOSED)
                return false;

    // There should be a direction where the path is walled, and another
    // where it is open
    bool up_walk = IS_WALKABLE(level[pos.x - 1][pos.y]);
    bool down_walk = IS_WALKABLE(level[pos.x + 1][pos.y]);
    bool left_walk = IS_WALKABLE(level[pos.x][pos.y - 1]);
    bool right_walk = IS_WALKABLE(level[pos.x][pos.y + 1]);

    return (!up_walk && !down_walk && left_walk && right_walk) ||
           (up_walk && down_walk && !left_walk && !right_walk);
}


/**
 * Marks all areas accessible from a point and not yet marked with a specific
 * value.
 * @param map The map to mark
 * @param starting_pos Starting point
 * @param value Value to mark with
 * @warning This function will not traverse any points that are already marked
 * with the given value.
 */
static void flood(int dlvl, int *map, Coord starting_pos, int value)
{
    map[starting_pos.x * LEVEL_WIDTH + starting_pos.y] = value;

    for (int i_neighbor = 0; i_neighbor < 8; i_neighbor++) {
        Coord neighbor = get_neighbor(starting_pos, i_neighbor);
        if (valid_coordinates(neighbor) &&
            POTENTIALLY_WALKABLE(maps[dlvl][neighbor.x][neighbor.y]) &&
            map[neighbor.x * LEVEL_WIDTH + neighbor.y] != value)
            flood(dlvl, map, neighbor, value);
    }
}


/**
 * Find connected components in the map
 * @param dlvl The level in which to find components
 * @param components An allocated array of size LEVEL_HEIGHT*LEVEL_WIDTH in
 * which the component map will be stored
 * @return the number of components
 */
static int find_connected_components(int dlvl, int *components)
{
    int i_component = 0;

    for (int i = 0; i < LEVEL_HEIGHT * LEVEL_WIDTH; i++)
        components[i] = -1;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (POTENTIALLY_WALKABLE(maps[dlvl][i_x][i_y]) &&
                components[i_x * LEVEL_WIDTH + i_y] == -1) {
                // Walkable and not yet part of any connected component, run
                // flood algorithm
                flood(dlvl, components, (Coord) {i_x, i_y}, i_component);
                i_component++;
            }
        }
    }

    return i_component;
}


/**
 * Checks if removing a tile would connect two components sideways (not
 * diagonally).
 * @param components_map The components map, as generated by
 * `find_connected_components`.
 * @param pos The position of the tile to check
 * @param component If set to a >= 0 value, will only return true if the tile
 * connects this component on one side.
 */
static bool separates_components(int *components_map, Coord pos, int component)
{
    int first_component = -1;

    // Only check the first 4 neighbors, i.e the sides
    for (int i_neighbor = 0; i_neighbor < 4; i_neighbor++) {
        Coord neighbor = get_neighbor(pos, i_neighbor);
        if (!valid_coordinates(neighbor))
            continue;

        int val = components_map[neighbor.x * LEVEL_WIDTH + neighbor.y];
        if (val != -1) {
            if (first_component == -1)
                first_component = val;
            else if (val != first_component) {
                if (component < 0)
                    return true;
                return first_component == component || val == component;
            }
        }
    }

    return false;
}


/**
 * Replaces a wall with a door (or a portcullis if possible).
 * @param dlvl The level on which to perform the replacement.
 * @param pos The coordinates of the door (or center of the portcullis).
 */
static void open_wall(int dlvl, Coord pos)
{
    /* Note here: if we're on a maintenance level and there are pipes, the
     * orientation will never be predicted correctly. But who installs a
     * portcullis in piping ? */
    int orientation = get_orientation(dlvl, pos, T_WALL);

    if (orientation == 0 || rand_int(1, 100) > portcullis_chance) {
        if (rand_int(1, 100) <= open_door_chance) {
            maps[dlvl][pos.x][pos.y] = T_DOOR_OPEN;
        } else {
            maps[dlvl][pos.x][pos.y] = T_DOOR_CLOSED;
        }
    }

    Coord increment_direction, decrement_direction;

    if (orientation == 1) { // Horizontal
        increment_direction = (Coord) {0, 1};
        decrement_direction = (Coord) {0, -1};
    } else {
        increment_direction = (Coord) {1, 0};
        decrement_direction = (Coord) {-1, 0};
    }

    Coord incremented = coord_add(pos, increment_direction);
    Coord decremented = coord_add(pos, decrement_direction);

    if (valid_coordinates(incremented) &&
        can_place_door(maps[dlvl], incremented) &&
        valid_coordinates(decremented) &&
        can_place_door(maps[dlvl], decremented)) {
        maps[dlvl][incremented.x][incremented.y] = T_PORTCULLIS_DOWN;
        maps[dlvl][pos.x][pos.y] = T_PORTCULLIS_DOWN;
        maps[dlvl][decremented.x][decremented.y] = T_PORTCULLIS_DOWN;

        // Place a lever to be accessible from the stairs down
        int *components = malloc(LEVEL_HEIGHT * LEVEL_WIDTH * sizeof(int));
        bool *mask = malloc(LEVEL_HEIGHT * LEVEL_WIDTH * sizeof(bool));

        find_connected_components(dlvl, components);
        Coord stairs_up;
        int found_stairs = find_tile(&stairs_up, dlvl, true, T_STAIRS_UP);
        int main_component;

        if (found_stairs)
            main_component = components[stairs_up.x * LEVEL_WIDTH +
                                        stairs_up.y];
        else
            main_component = 0;

        for (int i = 0; i < LEVEL_HEIGHT * LEVEL_WIDTH; i++)
            mask[i] = (components[i] == main_component);

        Coord lever;
        find_tile_mask(&lever, mask);

        maps[dlvl][lever.x][lever.y] = T_LEVER;
        register_lever(dlvl, lever, dlvl, pos);

        free(components);
        free(mask);
    }
}

/**
 * Maybe connects components.
 * @param dlvl
 * @param components
 */
static void connect_components(int dlvl)
{
    int *components = malloc(LEVEL_HEIGHT * LEVEL_WIDTH * sizeof(int));
    int connects[LEVEL_HEIGHT][LEVEL_WIDTH];

    int max_attempts = ndn(2, find_connected_components(dlvl, components) / 2);
    Coord stairs_up;
    int found_stairs = find_tile(&stairs_up, dlvl, true, T_STAIRS_UP);

    int main_component;
    if (found_stairs)
        main_component = components[stairs_up.x * LEVEL_WIDTH + stairs_up.y];
    else
        main_component = 0;

    for (int attempt = 0; attempt < max_attempts; attempt++) {
        int possibles = 0;
        for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
            for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
                connects[i_x][i_y] = separates_components(components,
                                                          (Coord) {i_x, i_y},
                                                          main_component);
                if (connects[i_x][i_y])
                    possibles++;
            }
        }

        if (possibles == 0)
            break;

        int selected = rand_int(0, possibles - 1);
        Coord pos = (Coord) {0, 0};
        bool found = false;

        for (int i_x = 0; i_x < LEVEL_HEIGHT && !found; i_x++) {
            for (int i_y = 0; i_y < LEVEL_WIDTH && !found; i_y++) {
                if (connects[i_x][i_y]) {
                    if (selected == 0) {
                        pos.x = i_x;
                        pos.y = i_y;
                        found = true;
                    }
                    selected--;
                }
            }
        }

        if (!found) {
            print_to_log("Something went wrong in connect_components().\n");
            break;
        }

        open_wall(dlvl, pos);
        // Update components map
        find_connected_components(dlvl, components);
    }

    free(components);
}


/**
 * Checks if a point is in a rectangle
 * @param point Position of the point
 * @param rectangle_pos, size Position and size of the rectangle
 */
static bool in_rectangle(Coord point, Coord rectangle_pos, Coord size)
{
    return (point.x >= rectangle_pos.x && point.x <= rectangle_pos.x + size.x &&
            point.y >= rectangle_pos.y && point.y <= rectangle_pos.y + size.y);
}


/**
 * Returns the position of the upper left corner of a valid rectangular area
 * on a level (i.e. an area that does not contain any stairs). The area will
 * not touch an outside wall (i.e. it won't extend to x=0, y=0, x=HEIGHT-1 or
 * y=WIDTH-1).
 * @param dlvl
 * @param height, width Size of the rectangular area
 * @param pos Pointer to where the position will be stores
 * @return Whether such an area was found
 */
static bool select_valid_rectangle(int dlvl, int height, int width, Coord *pos)
{
    // TODO: write this in a smart way :)
    Coord stairs_up, stairs_down;
    int has_stairs_up = find_tile(&stairs_up, dlvl, true, T_STAIRS_UP) > 0;
    int has_stairs_down =
            find_tile(&stairs_down, dlvl, true, T_STAIRS_DOWN) > 0;

    for (int attempt = 0; attempt < 100; attempt++) {
        pos->x = rand_int(1, LEVEL_HEIGHT - height - 2);
        pos->y = rand_int(1, LEVEL_WIDTH - width - 2);

        if ((!has_stairs_down ||
             !in_rectangle(stairs_down, *pos, (Coord) {height, width})) &&
            (!has_stairs_up ||
             !in_rectangle(stairs_up, *pos, (Coord) {height, width})))
            return true;
    }

    return false;
}


/**
 * Add the administrator's room to a level
 */
static void make_administator(int dlvl)
{
    const int size_x = 10;
    const int size_y = 10;

    Coord room_pos;
    if (!select_valid_rectangle(dlvl, size_x, size_y, &room_pos)) {
        print_to_log("Could not find a room for the administrator!\n");
        return;
    }

    // Clear it out
    for (int i_x = 0; i_x <= size_x; i_x++)
        for (int i_y = 0; i_y <= size_y; i_y++)
            maps[dlvl][room_pos.x + i_x][room_pos.y + i_y] = T_FLOOR;

    // Make collapsed area
    for (int i_x = 0; i_x <= size_x; i_x++)
        for (int i_y = 0; i_y <= size_y; i_y++)
            if (i_x == 0 || i_x == size_x || i_y == 0 || i_y == size_y)
                maps[dlvl][room_pos.x + i_x][room_pos.y + i_y] = T_COLLAPSED;

    // Make walls
    for (int i_x = 1; i_x <= size_x - 1; i_x++)
        for (int i_y = 1; i_y <= size_y - 1; i_y++)
            if (i_x == 1 || i_x == size_x - 1 || i_y == 1 || i_y == size_y - 1)
                maps[dlvl][room_pos.x + i_x][room_pos.y + i_y] = T_WALL;

    // Add a door
    int door_pos = rand_int(0, size_x - 5);
    int offset = rand_int(0, 1) * (size_y - 2);
    maps[dlvl][room_pos.x + 3 + door_pos][room_pos.y + 1 + offset] =
            T_DOOR_CLOSED;

    // Add a lever on a walkable tile
    Coord lever_position;
    lever_position.x = room_pos.x + rand_int(3, size_x - 3);
    lever_position.y = room_pos.y + rand_int(3, size_y - 3);
    maps[dlvl][lever_position.x][lever_position.y] = T_LEVER;

    //TODO: add a zoo
}


/**
 * Add the archmage's garden to a level.
 */
static void make_archmage(int dlvl)
{
    const int garden_width = 30;
    const int garden_height = 15;
    /* Feature probabilities, cumulative */
    const int fungus_prob = 20;
    const int tree_prob = 25;
    const int fountain_prob = 28;

    Coord pos;
    pos.x = rand_int(1, LEVEL_HEIGHT - garden_height - 2);
    pos.y = rand_int(1, LEVEL_WIDTH - garden_width - 2);

    // Draw an ellipse
    for (int i_x = 0; i_x < garden_height; i_x++) {
        double x_prime = 1.0 - 2 * (i_x + 0.5) / (double) garden_height;
        double width = garden_width * sqrt(1 - pow(x_prime, 2));
        int offset = (int) round((garden_width - width) / 2);

        for (int i_y = offset; i_y < garden_width - offset; i_y++) {
            int x = i_x + pos.x;
            int y = i_y + pos.y;

            if (IS_STAIRS(maps[dlvl][x][y]))
                continue;

            int selection = rand_int(1, 100);
            TileType new_type;
            if (selection <= fungus_prob)
                new_type = T_FUNGUS;
            else if (selection <= tree_prob)
                new_type = T_TREE;
            else if (selection <= fountain_prob)
                new_type = T_FOUNTAIN;
            else
                new_type = T_GRASS;

            maps[dlvl][x][y] = new_type;
        }
    }
}


/**
 * Replaces a wall with piping
 * @param dlvl Coordinates of the starting point
 */
void create_pipe(int dlvl, Coord starting_point)
{
    const int exhaust_prob = 5;
    const int stop_prob = 8;

    if (rand_int(1, 100) <= stop_prob)
        return;

    if (rand_int(1, 100) <= exhaust_prob)
        maps[dlvl][starting_point.x][starting_point.y] = T_PIPE_EXHAUST;
    else
        maps[dlvl][starting_point.x][starting_point.y] = T_PIPE;

    for (int i_neighbor = 0; i_neighbor < 8; i_neighbor++) {
        Coord neighbor = get_neighbor(starting_point, i_neighbor);
        if (valid_coordinates(neighbor) &&
            !is_outer_wall(neighbor) &&
            maps[dlvl][neighbor.x][neighbor.y] == T_WALL)
            create_pipe(dlvl, neighbor);
    }
}


/**
 * Replaces some walls with pipes and pipe exhausts.
 */
void make_maintenance_pipes(int dlvl)
{
    int starting_points = ndn(3, 5);
    int failsafe = 10;

    for (int i_start = 0; i_start < starting_points && failsafe; i_start++) {
        Coord starting;
        int found = find_tile(&starting, dlvl, true, T_WALL);

        if (!found)
            break;

        if (is_outer_wall(starting)) {
            failsafe--;
            continue;
        }

        create_pipe(dlvl, starting);
    }
}


/**
 * Generate a given level from the grid file
 * @param dlvl The level to generate
 */
void generate_level(int dlvl)
{
    TileType (*level_map)[LEVEL_WIDTH] = maps[dlvl];

    if (dlvl < 0 || dlvl > DLVL_MAX) {
        print_to_log("Asked to generate dlvl %d (max %d)\n", dlvl, DLVL_MAX);
        exit(0);
    }

    // Load the grid into a new level map
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            switch (grid[i_x][i_y]) {
            case GT_FLOOR:
                level_map[i_x][i_y] = T_FLOOR;
                break;
            case GT_UNREMOVABLE_WALL:
            case GT_WALL:
            case GT_SEPARATOR:
                level_map[i_x][i_y] = T_WALL;
                break;
            default:
                level_map[i_x][i_y] = T_WALL;
                print_to_log("Unknown grid tile type %d\n", grid[i_x][i_y]);
                break;
            }
        }
    }

    // Randomly keep/delete walls
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (grid[i_x][i_y] != GT_WALL)
                continue;

            if ((dlvl_types[dlvl] != DLVL_MAINTENANCE &&
                 rand_int(1, 10) <= 5) ||
                (dlvl_types[dlvl] == DLVL_MAINTENANCE && rand_int(1, 10) <= 2))
                replace_tiles_from(level_map, (Coord) {i_x, i_y}, GT_WALL,
                                   T_FLOOR);
            else
                replace_tiles_from(level_map, (Coord) {i_x, i_y}, GT_WALL,
                                   T_WALL);
        }
    }

    // Prune separators
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (grid[i_x][i_y] == GT_SEPARATOR) {
                if (surrounded_by_walkable(level_map, (Coord) {i_x, i_y}))
                    level_map[i_x][i_y] = T_FLOOR;
                else
                    level_map[i_x][i_y] = T_WALL;
            }
        }
    }

    // Add stairs
    Coord stair_down, stair_up;
    bool has_stair_down = false, has_stair_up = false;

    if (dlvl != DLVL_MAX - 1) {
        if (!find_tile(&stair_down, dlvl, 1, -1)) {
            print_to_log("Could not place stairs down on dlvl %d!\n", dlvl);
        } else {
            maps[dlvl][stair_down.x][stair_down.y] = T_STAIRS_DOWN;
            has_stair_down = true;
        }
    }

    int result;

    if (dlvl == 0) {
        result = find_tile(&stair_up, dlvl, 1, -1);
    } else {
        Coord previous_stairs;
        int found_previous_stairs = find_tile(&previous_stairs, dlvl - 1, 1,
                                              T_STAIRS_DOWN);
        if (!found_previous_stairs) {
            result = find_tile(&stair_up, dlvl, 1, -1);
        } else {
            result = find_closest(&stair_up, dlvl, previous_stairs, 1, -1);
        }
    }

    if (!result) {
        print_to_log("Could not place stairs up on dlvl %d!\n", dlvl);
    } else {
        maps[dlvl][stair_up.x][stair_up.y] = T_STAIRS_UP;
        has_stair_up = true;
    }

    // Connect some components
    connect_components(dlvl);

    // Check that the level is traversable
    if (has_stair_down && has_stair_up) {
        if (!can_walk(dlvl, stair_down, stair_up))
            make_path(level_map, stair_down, stair_up);
    }

    // Add doors
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            if (can_place_door(level_map, (Coord) {i_x, i_y}))
                if ((!IS_WALKABLE(level_map[i_x][i_y]) && rand_int(0, 30) <= 2)
                    ||
                    (IS_WALKABLE(level_map[i_x][i_y]) &&
                     rand_int(0, 40) <= 1)) {
                    open_wall(dlvl, (Coord) {i_x, i_y});
                }

    // TODO: add traps & features

    if (dlvl_types[dlvl] == DLVL_MAINTENANCE) {
        make_maintenance_pipes(dlvl);
    }

    // TODO: add patches of fungus/grass

    // Potentially add collapsed area
    if (dlvl != DLVL_MAX - 1 && rand_int(1, 100) <= 33 &&
        (dlvl_types[dlvl] == DLVL_NORMAL ||
         dlvl_types[dlvl] == DLVL_MAINTENANCE)) {
        bool *blob = calloc((size_t) chasm_height * chasm_width, sizeof(bool));
        make_blob(blob, chasm_height, chasm_width);
        for (int attempt = 0; attempt < 5; attempt++) {
            Coord blob_pos;
            blob_pos.x = rand_int(1, LEVEL_HEIGHT - chasm_height - 2);
            blob_pos.y = rand_int(1, LEVEL_WIDTH - chasm_width - 2);
            if (has_stair_up && has_stair_down &&
                can_walk_blob(blob, chasm_height, chasm_width, blob_pos, dlvl,
                              stair_down, stair_up)) {
                blit_blob(dlvl, blob, chasm_height, chasm_width, blob_pos,
                          T_COLLAPSED);
            }
        }
    }

    // Make special levels. TODO: markets, banks, barracks
    switch (dlvl_types[dlvl]) {
    case DLVL_ADMINISTRATOR:
        make_administator(dlvl);
        break;
    case DLVL_ARCHMAGE:
        make_archmage(dlvl);
        break;
    default:
        break;
    }

    // Set visibility map
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            visibility_map[dlvl][i_x][i_y] = TS_UNDISCOVERED;

    visited[dlvl] = false;
}
