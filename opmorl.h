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

#include <ncurses.h>

#include "linkedlist.h"

#define DEBUG

#define VERSION 0.01
#define STRING_V "2 prealpha"
#define DEFAULT 0
#define DEFAULT_BACKCOLOR -1
#define DEFAULT_FORECOLOR -1

#define LEVELS 25

/* Structs */


typedef enum {
	CLR_DEFAULT, /* COLOR_PAIR(0) is the default back/fore ground colors */
	CLR_WHITE
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

	char name[50];

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

	char name[50];

	int posx, posy, level;
	int attack;
	int life_points;
	int flags; /* Such as invisible, flying ... */

	char symbol;
	Color color; /* to be used with COLOR_PAIR(color) */
} Monster;

typedef enum {
	T_WALL,
	T_CORRIDOR,
	T_OPEN_DOOR,
	T_CLOSED_DOOR,
	T_FLOOR,
	T_STAIRS
} TileType;

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
	int level; /* The depth in the dungeon or whatever you may call it */
	int dexterity, strength, constitution, intelligence, wisdom, charisma; /* Stats */

	Object inventory[52];
	int gold;
	Color color;
} Player;

/* Prototypes */

void init_ncurses();
void init_game();
void init_colors();
void game_loop();
void exit_game();
void exit_ncurses();

void create_lvl(int);

void get_input();

int pline(char *);
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

void move_rodney(int, int);
void go_down();

/* Globals */

TileType lvl_map[LEVELS][21][80];
Player rodney;

LinkedList *o_list;
LinkedList *m_list;