/*
 *  map.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

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
                {false, true,  '.', CLR_DARKGRAY},  // collapsed
                {true,  true,  '^', CLR_DEFAULT},   // locked trapdoor
                {false, true,  '-', CLR_DEFAULT},   // open trapdoor
                {true,  true,  '/', CLR_DEFAULT},   // lever
                {false, false, 'W', CLR_YELLOW},    // pipe (special sym)
                {false, false, 'o', CLR_YELLOW},    // pipe exhaust
                {true,  true,  ';', CLR_GREEN},     // grass
                {true,  true,  ';', CLR_DEFAULT},   // fungus
                {false, false, 'T', CLR_DEFAULT},   // tree
                {true,  true,  '}', CLR_BLUE},      // fountain
                {true,  true,  '.', CLR_STEELBLUE}, // portcullis up
                {false, true,  '#', CLR_STEELBLUE}, // portcullis down
                {true,  false, '*', CLR_LIGHTGRAY}, // rubble
        };


/** Size of a chasm */
const int chasm_width = 40;
const int chasm_height = 15;

/** Size of a patch area */
const int patch_width = 7;
const int patch_height = 4;


/**
 * Checks if the coordinates are valid (ie in the map)
 */
static bool valid_coordinates(int x, int y)
{
    return (x >= 0 && x < LEVEL_HEIGHT && y >= 0 && y < LEVEL_WIDTH);
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

    if (((!looking_for_specific_type &&
          IS_WALKABLE(maps[dlvl][pos.x][pos.y])) ||
         (looking_for_specific_type &&
          maps[dlvl][pos.x][pos.y] == tile_type)) &&
        (!can_have_mon ||
         (find_mon_at(dlvl, pos) == NULL && pos.x != rodney.pos.x &&
          pos.y != rodney.pos.y))) {
        return true;
    }
    return false;
}

/**
 * Returns a random walkable tile on the specified map level, fitting given
 * criteria.
 * @param dlvl the dungeon level on which to perform the search
 * @param coords pointer where the location of a tile will be stored.
 * @param can_have_mon if false, the tile will not have a monster on it.
 * @param tile_type if set in the range [0, NB_TILE_TYPES-1], will only look
 * for tiles of that type. The tile type doesn't have to be walkable. Otherwise,
 * will return a random *walkable* tile.
 * @returns the number of available tiles satisfying the constraints. Can be 0.
 */
int
find_tile(int dlvl, Coord *coords, bool can_have_mon, int tile_type)
{
    bool avail[LEVEL_HEIGHT][LEVEL_WIDTH];
    int nb_avail = 0;
    int i_selected;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (fits_criteria(dlvl, (Coord) {i_x, i_y}, can_have_mon,
                              tile_type)) {
                avail[i_x][i_y] = true;
                nb_avail++;
            } else {
                avail[i_x][i_y] = false;
            }
        }
    }

    if (nb_avail == 0)
        return 0;

    i_selected = rand_int(0, nb_avail - 1);
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if (avail[i_x][i_y]) {
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
    return 0;
}


/**
 * Finds the closest (walkable) tile to a specified tile.
 * @param dlvl The depth level to search on
 * @param coords A pointer to a Coord struct where the coordinates will be
 * stored
 * @param can_have_mon Whether the tile can have a monster on it
 * @param tile_type If in [0, NB_TILE_TYPES-1], specifies a tile type
 * @param near The coordinates around which we look
 * @return Whether a tile was found.
 */
bool find_closest(int dlvl, Coord *coords, bool can_have_mon, int tile_type,
                  Coord near)
{
    for (int radius = 0; radius < LEVEL_WIDTH; radius++) {
        for (int i_x = near.x - radius; i_x <= near.x + radius; i_x++) {
            for (int i_y = near.y - radius; i_y <= near.y + radius; i_y++) {
                if (i_x != near.x - radius && i_x != near.x + radius &&
                    i_y != near.y - radius && i_y != near.y + radius)
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

    stack[0] = from;

    while (stack_pointer >= 0) {
        Coord cur_root = stack[stack_pointer];
        stack_pointer--;

        if (cur_root.x == to.x && cur_root.y == to.y)
            return true;

        for (int i_neighbor = 0; i_neighbor < 8; i_neighbor++) {
            Coord cur = get_neighbor(cur_root, i_neighbor);
            if (!valid_coordinates(cur.x, cur.y))
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
                maps[dlvl][i_x][i_y] == T_CLOSED_DOOR) {
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
                 maps[dlvl][i_x][i_y] == T_CLOSED_DOOR)) {
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
 * @param monsters_block If set to true, monsters will count as unwalkable
 * (opaque) tiles. This is useful for ranged weapons calculation.
 * @param block Pointers that will be updated with the coordinates
 * of the blocking tile (if not set to NULL).
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
                if (!valid_coordinates(cur.x, cur.y))
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
static int count_neighbors(bool *array, int height, int width, int x,
                           int y)
{
    int count = 0;

    for (int neighbor_x = -1; neighbor_x <= 1; neighbor_x++) {
        for (int neighbor_y = -1; neighbor_y <= 1; neighbor_y++) {
            if (neighbor_x == 0 && neighbor_y == 0)
                continue;

            int check_x = x + neighbor_x;
            if (check_x < 0 || check_x >= height)
                continue;

            int check_y = y + neighbor_y;
            if (check_y < 0 || check_y >= width)
                continue;

            if (array[check_x * width + check_y])
                count++;
        }
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
                                                     height, width, i_x, i_y);

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
 * level.
 * @param level A level map in which to perform the replacement
 * @param from_x, from_y The coordinates from which to start the replacement
 * @param to_type The new tile type
 */
void replace_tiles_from(TileType (*level)[LEVEL_WIDTH], Coord from,
                        int source_type, TileType to_type)
{
    if (grid[from.x][from.y] != source_type || level[from.x][from.y] == to_type)
        return;

    level[from.x][from.y] = to_type;

    if (from.x != 0)
        replace_tiles_from(level, (Coord) {from.x - 1, from.y}, source_type,
                           to_type);
    if (from.x != LEVEL_HEIGHT - 1)
        replace_tiles_from(level, (Coord) {from.x + 1, from.y}, source_type,
                           to_type);

    if (from.y != 0)
        replace_tiles_from(level, (Coord) {from.x, from.y - 1}, source_type,
                           to_type);
    if (from.y != LEVEL_WIDTH - 1)
        replace_tiles_from(level, (Coord) {from.x, from.y + 1}, source_type,
                           to_type);
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
    if (pos.x == 0 || pos.x == LEVEL_HEIGHT - 1 || pos.y == 0
        || pos.y == LEVEL_WIDTH - 1)
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
    if (pos.x == 0 || pos.x == LEVEL_HEIGHT - 1 || pos.y == 0
        || pos.y == LEVEL_WIDTH - 1)
        return false;

    // We shouldn't be touching another door
    for (int i_x = pos.x - 1; i_x <= pos.x + 1; i_x++)
        for (int i_y = pos.y - 1; i_y <= pos.y + 1; i_y++)
            if (level[i_x][i_y] == T_OPEN_DOOR ||
                level[i_x][i_y] == T_CLOSED_DOOR)
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
    int has_stairs_up = find_tile(dlvl, &stairs_up, true, T_STAIRS_UP) > 0;
    int has_stairs_down =
            find_tile(dlvl, &stairs_down, true, T_STAIRS_DOWN) > 0;

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
            T_CLOSED_DOOR;
}


/**
 * Add the archmage's garden to a level
 */
static void make_archmage(int dlvl)
{

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

            if (rand_int(0, 10) <= 4)
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
        if (!find_tile(dlvl, &stair_down, 1, -1)) {
            print_to_log("Could not place stairs down on dlvl %d!\n", dlvl);
        } else {
            maps[dlvl][stair_down.x][stair_down.y] = T_STAIRS_DOWN;
            has_stair_down = true;
        }
    }

    int result;

    if (dlvl == 0) {
        result = find_tile(dlvl, &stair_up, 1, -1);
    } else {
        Coord previous_stairs;
        int found_previous_stairs = find_tile(dlvl - 1, &previous_stairs, 1,
                                              T_STAIRS_DOWN);
        if (!found_previous_stairs) {
            result = find_tile(dlvl, &stair_up, 1, -1);
        } else {
            result = find_closest(dlvl, &stair_up, 1, -1, previous_stairs);
        }
    }

    if (!result) {
        print_to_log("Could not place stairs up on dlvl %d!\n", dlvl);
    } else {
        maps[dlvl][stair_up.x][stair_up.y] = T_STAIRS_UP;
        has_stair_up = true;
    }

    // Check that the level is traversable
    if (has_stair_down && has_stair_up) {
        if (!can_walk(dlvl, stair_down, stair_up))
            make_path(level_map, stair_down, stair_up);
    }

    // Add doors
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            if (can_place_door(level_map, (Coord) {i_x, i_y}) &&
                level_map[i_x][i_y] != T_STAIRS_DOWN &&
                level_map[i_x][i_y] != T_STAIRS_UP)
                if ((!IS_WALKABLE(level_map[i_x][i_y]) && rand_int(0, 30) <= 2)
                    ||
                    (IS_WALKABLE(level_map[i_x][i_y]) &&
                     rand_int(0, 40) <= 1)) {
                    if (rand_int(0, 5) == 0)
                        level_map[i_x][i_y] = T_OPEN_DOOR;
                    else
                        level_map[i_x][i_y] = T_CLOSED_DOOR;
                }

    // TODO: add traps & features
    // TODO: add pipes on maintenance levels
    // TODO: add patches of fungus/grass

    // Potentially add collapsed area
    if (dlvl != DLVL_MAX - 1 && rand_int(1, 100) <= 40 &&
        (dlvl_types[dlvl] == DLVL_NORMAL ||
         dlvl_types[dlvl] == DLVL_MAINTENANCE)) {
        bool *blob = calloc((size_t) chasm_height * chasm_width, sizeof(bool));
        make_blob(blob, chasm_height, chasm_width);
        for (int attempt = 0; attempt < 5; attempt++) {
            Coord blob_pos;
            blob_pos.x = rand_int(0, LEVEL_HEIGHT - chasm_height - 1);
            blob_pos.y = rand_int(0, LEVEL_WIDTH - chasm_width - 1);
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
