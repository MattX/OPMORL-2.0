/*
 *  opmorl.h
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

/* This header is to be included in every other file for now*/

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include <ncurses.h>

#include "linkedlist.h"

#define DEBUG

#define VERSION 0.01
#define STRING_V "2 prealpha"
#define DEFAULT 0
#define DEFAULT_BACKCOLOR -1
#define DEFAULT_FORECOLOR -1

#define LEVELS 3
#define LEVEL_WIDTH 80
#define LEVEL_HEIGHT 21

#define MAX_NAME 50

#define LOGFILE_PATH "opmorl.log"

/* Structs */


typedef enum {
    CLR_DEFAULT = 0, /* COLOR_PAIR(0) is the default back/fore ground colors */
    CLR_WHITE = 1,
    CLR_YELLOW = 2
} Color;
/* Here we have stuff like CLR_ORANGE or whatever, that we have initialized in init_color, and then we	*
 * just have, before printing a monster/object, to call attron(COLOR_PAIR(obj->color)); and attroff().	*/


typedef enum {
	OT_SWORD,
	OT_SHIELD,
	OT_POTION,
	OT_WAND,
	OT_RING,
	OT_BODY_ARMOR,
	OT_HELM,
	OT_FOOD,
	OT_SCROLL /* We are free not to implement all of them right now */
} ObjectType; /* Object type */

typedef struct Object{
	ObjectType type;

    char name[MAX_NAME];

	int is_gold; /* We treat this separately */
	int posx, posy, level; /* Coordinates : x, y, level (we want persistent levels) */
	int value; /* The monetary value for buying / selling the object */
	int damage; /* For everything that attacks, including wands */
	//	Effect effect; /* For potions */
	int shots_left; /* For wands */
	int flags; /* Such as invisible... */

	char symbol; /* To display */
	Color color; /* to be used with COLOR_PAIR(color) */
} Object;


typedef enum {
	EMPTY, FOR, NOW
} Mtype; /* Monster type */

typedef struct Monster {
	Mtype type;

    char name[MAX_NAME];

	int posx, posy, level;
	int attack;
	int life_points;
	int flags; /* Such as invisible, flying ... */

	char symbol;
	Color color; /* to be used with COLOR_PAIR(color) */
} Monster;

typedef enum {
    T_WALL = 0x1,
    T_CORRIDOR = 0x2,
    T_OPEN_DOOR = 0x4,
    T_CLOSED_DOOR = 0x8,
    T_FLOOR = 0x10,
    T_STAIRS_UP = 0x20,
    T_STAIRS_DOWN = 0x40,
    T_GROUND = 0x80,
} TileType;

#define T_ANY (~0)
#define T_WALKABLE (T_CORRIDOR | T_OPEN_DOOR | T_FLOOR | T_STAIRS_UP | T_STAIRS_DOWN)

typedef enum {
	C_SAMURAI,
	C_WARRIOR,
	C_ARCHER,
    C_VALKYRIE
} PlayerClass;

typedef struct {
	PlayerClass pclass;
	int posx, posy; /* Position */
	int explevel, exp; /* Experience stuff */
	int hp, max_hp;
    int dlvl; /* The depth in the dungeon or whatever you may call it */
	int dexterity, strength, constitution, intelligence, wisdom, charisma; /* Stats */

	Object inventory[52];
	int gold;
    int score;
	Color color;
} Player;

/* Prototypes */

void init_ncurses();
void init_game();
void init_colors();
void game_loop();
void exit_game();
void exit_ncurses();

int find_floor_tile(int, int *, int *, int, int);

void create_level(int);

char get_input();

void pline(char *, ...);

int yes_no(char *, ...);
void display_everything();
void display_map();
void display_stats();

void add_object(Object *);
void add_monster(Monster *);
void rm_mon_at(int, int, int);
void rm_obj_at(int, int, int);
Monster *find_mon_at(int, int, int);
Object *find_obj_at(int, int, int);

int rand_int(int, int);

int min(int, int);

int max(int, int);

int sign(int);

int move_rodney(int, int);

int use_stairs(int);

void process_turn(char c);

/* Globals */

TileType lvl_map[LEVELS][LEVEL_HEIGHT][LEVEL_WIDTH];
Player rodney;

LinkedList *o_list;
LinkedList *m_list;

/* Whether there is something displayed on the status line already */
int line_displayed;
int last_col;
FILE *log_file;

void print_to_log(char *format, ...);
