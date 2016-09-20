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

#define INVENTORY_SIZE 52

#define MAX_NAME 50
#define MAX_DESCR 200

#define LOGFILE_PATH "opmorl.log"

/* Structs */


typedef enum e_color
{
    CLR_DEFAULT = 0, /* COLOR_PAIR(0) is the default back/fore ground colors */
    CLR_WHITE = 1,
    CLR_YELLOW = 2
} Color;
/* Here we have stuff like CLR_ORANGE or whatever, that we have initialized in init_color, and then we	*
 * just have, before printing a monster/object, to call attron(COLOR_PAIR(obj->color)); and attroff().	*/


/****** OBJECTS & object mixins ******/

// Magic classes
#define NB_MAGIC_CLASSES 5
int magic_class_weakness[NB_MAGIC_CLASSES][NB_MAGIC_CLASSES];
extern char *magic_class_names[NB_MAGIC_CLASSES];


#define NB_OT 8
// This is a flag field because mixins need to indicate compatibility with objects (and in general
// to allow filtering).
typedef enum {
    OT_MONEY = 0x01,
    OT_MELEE = 0x02,
    OT_POTION = 0x04,
    OT_WAND = 0x08,
    OT_TOOL = 0x10,
    OT_BODY_ARMOR = 0x20,
    OT_HELM = 0x40,
    OT_FOOD = 0x80,
} ObjectClassFlag; /* Object type */

#define OT_ALL (~0)

typedef struct
{
    char *possible_names; /* comma separated */

    ObjectClassFlag o_class_flag;
    char symbol;
} ObjectClass;

typedef struct
{
    ObjectClass *class;

    char name[MAX_NAME];
    int value;
    int power;

    int mixin1;
    int mixin2;
    int magic_class;
    Color color;
} ObjectType;


typedef enum
{
    MT_BG_INT, MT_BG_STR, MT_BG_DEX, MT_BG_HP, MT_BG_REGEN, MT_BG_SPEED, MT_BG_WAKE, MT_BG_TELEPATHY, MT_BG_ID,
    MT_BG_EMERGPORT,
    MT_AT_SMALL, MT_AT_LARGE, MT_AT_SPLIT, MT_AT_CRITICAL, MT_AT_MATCH, MT_AT_FREEZE, MT_AT_EXP, MT_AT_RAGE,
    MT_DF_SMALL, MT_DF_CRITICAL, MT_DF_WEAKNESS, MT_DF_BLOWBACK, MT_DF_REFLECTION, MT_DF_MELEE,
    MT_US_TP, MT_US_LEVELPORT, MT_US_MAP, MT_US_HP, MT_US_MAX_HP, MT_US_INT, MT_US_STR, MT_US_DEX, MT_US_OPEN,
} Mixin_type; // To be able to quickly check a mixin


typedef struct
{
    Mixin_type id;
    int compatible_classes;
    char *descr;
} Mixin;


typedef struct s_object
{
    const ObjectType *type;

    char name[MAX_NAME];

	int posx, posy, level; /* Coordinates : x, y, level (we want persistent levels) */
    int uses_left; /* For wands */
    int cooldown;
	int flags; /* Such as invisible... */
    int amount;
} Object;

void make_objects();

void add_level_objects(int level);

/****** MONSTERS ******/


typedef struct
{
} Mtype; /* Monster type */

typedef enum
{
    MT_INVISIBLE,
    MT_ASLEEP,
    MT_FROZEN,
} Mflags;

typedef struct s_monster
{
    Mtype type;

	int posx, posy, level;
	int life_points;
    int move_timeout; /* How much time before unfreezing/waking/etc. */
    Mflags flags; /* Such as invisible, flying ... */

	char symbol;
	Color color; /* to be used with COLOR_PAIR(color) */
} Monster;


/************/


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


typedef struct {
	int posx, posy; /* Position */
	int explevel, exp; /* Experience stuff */
	int hp, max_hp;
    int dlvl; /* The depth in the dungeon or whatever you may call it */
	int dexterity, strength, constitution, intelligence, wisdom, charisma; /* Stats */

    Object *inventory[INVENTORY_SIZE];
    Object *wielded;
    Object *helm;
    Object *body_armor;
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

void clear_msg_line();
int yes_no(char *, ...);
void display_everything();
void display_map();
void display_stats();

Object *select_object(LinkedList *objects);

void add_object(Object *);
void add_monster(Monster *);
void rm_mon_at(int, int, int);
Monster *find_mon_at(int, int, int);

LinkedList *find_objs_at(int, int, int);

int rand_int(int, int);
int min(int, int);
int max(int, int);
int sign(int);

void strncpy_pad(char *dest, const char *src, size_t n);

int move_rodney(int, int);

int use_stairs(int);

void show_env_messages();
void process_turn(char c);

void init_mixins();

int letter_to_slot(char);

char slot_to_letter(int);

int add_to_inventory(Object *obj);

int pickup();

int drop();

int dump_inventory();

/* Globals */

TileType lvl_map[LEVELS][LEVEL_HEIGHT][LEVEL_WIDTH];
Player rodney;
int turn;

LinkedList *o_list;
LinkedList *m_list;

/* Whether there is something displayed on the status line already */
int line_displayed;
int last_col;
FILE *log_file;

void print_to_log(char *format, ...);
