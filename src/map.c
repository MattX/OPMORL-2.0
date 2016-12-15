/*
 *  map.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"


/* Map tile definitions
 * MUST BE in the same order as tile definitions in opmorl.h */
struct s_tile_type tile_types[NB_TILE_TYPES] =
        {
                {false, '#', CLR_DEFAULT},   // wall
                {true,  '.', CLR_DEFAULT},   // corridor
                {true,  '-', CLR_DEFAULT},   // open door
                {false, '+', CLR_DEFAULT},   // closed door
                {true,  '.', CLR_DEFAULT},   // floor
                {true,  '<', CLR_DEFAULT},   // stairs down
                {true,  '>', CLR_DEFAULT},   // stairs up
                {false, ' ', CLR_DEFAULT},   // ground
                {false, ' ', CLR_DEFAULT},   // collapsed
                {true,  '^', CLR_DEFAULT},   // closed trapdoor
                {false, ' ', CLR_DEFAULT},   // open trapdoor
                {true,  '/', CLR_DEFAULT},   // lever
                {false, '=', CLR_YELLOW},    // pipe
                {false, 'o', CLR_YELLOW},    // pipe exhaust
                {true,  ';', CLR_GREEN},     // grass
                {true,  ';', CLR_DEFAULT},   // fungus
                {false, 'T', CLR_DEFAULT},   // tree
                {true,  '}', CLR_BLUE},      // fountain
                {true,  '.', CLR_WHITE},     // portcullis up
                {false, '#', CLR_WHITE},     // portcullis down
                {false, '*', CLR_DEFAULT},   // rubble
        };


/**
 * Returns a random walkable tile on the specified map level.
 * @dlvl the dungeon level on which to perform the search
 * @param coords pointers where the location of a tile will be stored, if one
 * was found.
 * @param can_have_mon if this flag is set to false, the returned tile will not
 * have a monster on it.
 * @param tile_type if set in the range [0, NB_TILE_TYPES-1], will only look
 * for tiles of that type. The tile type doesn't have to be walkable. Otherwise,
 * will return a random *walkable* tile.
 * @returns the number of available tiles satisfying the constraints.
 */
int
find_tile(int dlvl, Coord *coords, bool can_have_mon, int tile_type)
{
    bool avail[LEVEL_HEIGHT][LEVEL_WIDTH];
    int nb_avail = 0;
    int i_selected;
    bool looking_for_specific_type =
            tile_type >= 0 && tile_type < NB_TILE_TYPES;

    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++) {
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++) {
            if ((!looking_for_specific_type &&
                 IS_WALKABLE(maps[dlvl][i_x][i_y]))
                || (looking_for_specific_type &&
                    maps[dlvl][i_x][i_y] == tile_type)) {
                if (!can_have_mon &&
                    find_mon_at(dlvl, (Coord) {i_x, i_y}) != NULL)
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


        // TODO: add support for door generation
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
 * Checks if there is a walkable path from a point to another.
 * @param dlvl The level at which to perform the search
 * @param from Coordinates of the first point.
 * @param to Coordinates of the second point.
 * @return Whether there is a path from the first point to the second.
 */
bool can_walk(int dlvl, Coord from, Coord to)
{
    bool checked[LEVEL_HEIGHT][LEVEL_WIDTH];
    Coord stack[LEVEL_HEIGHT * LEVEL_WIDTH];
    int stack_pointer = 0;

    memset(checked, false, LEVEL_HEIGHT * LEVEL_WIDTH);

    stack[0] = from;

    while (stack_pointer >= 0) {
        Coord cur = stack[stack_pointer];
        stack_pointer--;
        checked[cur.x][cur.y] = true;

        if (cur.x == to.x && cur.y == to.y)
            return true;

        for (int x = cur.x - 1; x <= cur.x + 1; x++) {
            if (x < 0 || x >= LEVEL_HEIGHT)
                continue;
            for (int y = cur.y - 1; y <= cur.y + 1; y++) {
                if (y < 0 || y >= LEVEL_WIDTH)
                    continue;
                if (IS_WALKABLE(maps[dlvl][x][y]) && !checked[x][y]) {
                    stack_pointer++;
                    stack[stack_pointer].x = x;
                    stack[stack_pointer].y = y;
                }
            }
        }

    }

    return false;
}


/**
 * Checks whether a tile is visible from another tile on a given level.
 * Walkable tiles are considered see-through, other are opaque. The first
 * opaque tile on the way is considered visible. The algorithm is
 * invertible, that is, if point A is visible from point B, then B is
 * visible from A.
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

        if (!IS_WALKABLE(maps[dlvl][cur.x][cur.y]) ||
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

            else if (!visible &&
                     visibility_map[rodney.dlvl][i_x][i_y] != TS_UNDISCOVERED)
                visibility_map[rodney.dlvl][i_x][i_y] = TS_UNSEEN;

            else
                visibility_map[rodney.dlvl][i_x][i_y] = TS_UNDISCOVERED;
        }
}


/**
 * Runs Dijkstra's algorithm between two points.
 * @param dlvl The level on which to run the algorithm
 * @param from The source point coordinates
 * @param to The destination point coordinates
 * @param next Pointer in which the next position in the shortest path will be
 * stored if true is returned.
 * @param can_have_monst If set to false, only paths containing no monsters
 * will be considered
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
            Coord cur = tiles[rotation][i_tile];

            for (int i_x = cur.x - 1; i_x <= cur.x + 1; i_x++) {
                if (i_x < 0 || i_x > LEVEL_HEIGHT)
                    continue;

                for (int i_y = cur.y - 1; i_y <= cur.y + 1; i_y++) {
                    if (i_y < 0 || i_y > LEVEL_WIDTH)
                        continue;

                    if (!visited[i_x][i_y] &&
                        IS_WALKABLE(maps[dlvl][i_x][i_y]) &&
                        (can_have_monst ||
                         find_mon_at(dlvl, (Coord) {i_x, i_y}) == NULL)) {
                        tiles[new_rotation][nb_tiles[new_rotation]].x = i_x;
                        tiles[new_rotation][nb_tiles[new_rotation]].y = i_y;
                        prev[i_x][i_y] = cur;
                        visited[i_x][i_y] = true;
                        nb_tiles[new_rotation]++;
                    }

                    if (i_x == to.x && i_y == to.y) {
                        if (!visited[i_x][i_y]) {
                            return false;
                        }

                        // Backtrack to find tile to go to
                        Coord backtrack_cur = (Coord) {i_x, i_y};
                        Coord coming_from = cur;

                        while (backtrack_cur.x != from.x ||
                               backtrack_cur.y != from.y) {
                            coming_from = backtrack_cur;
                            backtrack_cur = prev[backtrack_cur.x][backtrack_cur.y];
                        }

                        *next = coming_from;

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

    // There should be a direction where the path is walled, and another
    // where it is open
    bool up_walkable = IS_WALKABLE(level[pos.x - 1][pos.y]);
    bool down_walkable = IS_WALKABLE(level[pos.x + 1][pos.y]);
    bool left_walkable = IS_WALKABLE(level[pos.x][pos.y - 1]);
    bool right_walkable = IS_WALKABLE(level[pos.x][pos.y + 1]);

    return (!up_walkable && !down_walkable && left_walkable &&
            right_walkable) ||
           (up_walkable && down_walkable && !left_walkable && !right_walkable);
}


/**
 * Generate walls and doors for a given level
 * @param dlvl The level to generate
 */
void make_layout_from_grid(int dlvl)
{
    TileType (*level_map)[LEVEL_WIDTH] = maps[dlvl];

    if (dlvl < 0 || dlvl > DLVL_MAX) {
        print_to_log("Asked to generate dlvl %d (max %d)\n", dlvl, DLVL_MAX);
        exit(0);
    }

    // Step 1: load the grid into a new level map
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

    // Step 2: randomly keep/delete walls
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

    // Step 3: prune separators
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

    // Step 4: Add stairs
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

    if (!find_tile(dlvl, &stair_up, 1, -1)) {
        print_to_log("Could not place stairs up on dlvl %d!\n", dlvl);
    } else {
        maps[dlvl][stair_up.x][stair_up.y] = T_STAIRS_UP;
        has_stair_up = true;
    }

    // Step 5: Check that the level is traversable
    if (has_stair_down && has_stair_up) {
        if (!can_walk(dlvl, stair_down, stair_up))
            make_path(level_map, stair_down, stair_up);
    }

    // Step 6: add doors
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            if (can_place_door(level_map, (Coord) {i_x, i_y}))
                if ((!IS_WALKABLE(level_map[i_x][i_y]) && rand_int(0, 30) <= 2)
                    ||
                    (IS_WALKABLE(level_map[i_x][i_y]) &&
                     rand_int(0, 40) <= 1)) {
                    if (rand_int(0, 2) == 0)
                        level_map[i_x][i_y] = T_OPEN_DOOR;
                    else
                        level_map[i_x][i_y] = T_CLOSED_DOOR;
                }

    // Step 8: Set visibility map
    for (int i_x = 0; i_x < LEVEL_HEIGHT; i_x++)
        for (int i_y = 0; i_y < LEVEL_WIDTH; i_y++)
            visibility_map[dlvl][i_x][i_y] = TS_UNDISCOVERED;

    visited[dlvl] = false;
}