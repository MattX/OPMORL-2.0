/*
 *  opmorl.h
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

/* This header is to be included in every other file for now */

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

// #include <ncurses.h>
#include <curses.h>

#include "linkedlist.h"

#define DEBUG

#define VERSION 0.01
#define STRING_V "dev"
#define DEFAULT 0
#define DEFAULT_BACKCOLOR -1
#define DEFAULT_FORECOLOR -1

#define LEVELS 16
#define LEVEL_WIDTH 80
#define LEVEL_HEIGHT 21

#define INVENTORY_SIZE 52
// Different object types
#define NB_OBJECTS 70

#define MAX_NAME 50
#define MAX_DESCR 200

#define LOGFILE_PATH "opmorl.log"

/* Structs */


typedef enum e_color
{
    CLR_DEFAULT = 0, /* COLOR_PAIR(0) is the default back/fore ground colors */
    CLR_WHITE,
    CLR_YELLOW,
    CLR_BLUE,
    CLR_MAGENTA,
    CLR_CYAN,
    CLR_RED,
    CLR_GREEN,
} Color;
/* Here we have stuff like CLR_ORANGE or whatever, that we have initialized in
 * init_color, and then we just have, before printing a monster/object, to call
 * attron(COLOR_PAIR(obj->color)); and attroff().	*/


/****** OBJECTS & object mixins ******/

// Magic classes
#define NB_MAGIC_CLASSES 6
extern bool magic_class_strengths[NB_MAGIC_CLASSES][NB_MAGIC_CLASSES];
extern char *magic_class_names[NB_MAGIC_CLASSES];
extern char *magic_class_adjectives[NB_MAGIC_CLASSES];
extern Color magic_class_colors[NB_MAGIC_CLASSES];

typedef enum
{
    MC_NEUTRAL = 0, MC_EVOK = 1, MC_EXOR = 2, MC_NECRO = 3, MC_ILLU = 4,
    MC_TRANS = 5,
} MagicClassTag;


#define NB_OT 8
// This is a flag field because mixins need to indicate compatibility with objects (and in general
// to allow filtering).
typedef enum
{
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

#define MAX_OBJCLASS 100
typedef struct
{
    char *possible_names; /* comma separated */

    ObjectClassFlag o_class_flag;
    char symbol;
    int prob;
} ObjectClass;

typedef struct
{
    ObjectClass *class;

    char base_name[MAX_NAME];
    int value;
    int power;

    int mixin1;
    int mixin2;
    bool mixin2_known;

    int magic_class;
    int magic_class_known;
    Color color;
} ObjectType;

#define MAX_MIXIN 100
typedef enum
{
    MT_BG_HP,
    MT_BG_REGEN,
    MT_BG_SPEED,
    MT_BG_WAKE,
    MT_BG_TELEPATHY,
    MT_BG_ID,
    MT_BG_EMERGPORT,
    MT_BG_SEE_INV,
    MT_AT_SMALL,
    MT_AT_LARGE,
    MT_AT_SPLIT,
    MT_AT_CRITICAL,
    MT_AT_MATCH,
    MT_AT_FREEZE,
    MT_AT_EXP,
    MT_AT_RAGE,
    MT_DF_SMALL,
    MT_DF_CRITICAL,
    MT_DF_WEAKNESS,
    MT_DF_BLOWBACK,
    MT_DF_REFLECTION,
    MT_DF_MELEE,
    MT_US_TP,
    MT_US_LEVELPORT,
    MT_US_MAP,
    MT_US_HP,
    MT_US_MAX_HP,
    MT_US_OPEN,
    MT_US_DIG,
    MT_US_ENCHANT,
    MT_US_CHMC,
    MT_US_ENLIGHTEN,
    MT_US_ID,
    MT_NONE = -1,
} Mixin_type; // To be able to quickly check a mixin


typedef struct
{
    Mixin_type id;
    int compatible_classes;
    char *descr;
    int prob;
    bool util;
} Mixin;


typedef struct s_object
{
    const ObjectType *type;

    int posx, posy, level; /* Coordinates : x, y, level (we want persistent levels) */
    int uses_left; /* For potions */
    int cooldown; /* Effect cooldown */
    int enchant; /* Enchant OR amount for money */
    int flags; /* Such as invisible... */
} Object;

ObjectType object_types[NB_OBJECTS];

void make_object_classes();

void add_level_objects(int level);

/****** MONSTERS ******/

#define MAX_NB_MONSTERS 200

// For attacks and other monster properties
typedef enum
{
    ATK_MELEE = 0x01,
    ATK_FREEZE = 0x02,
    ATK_FIRE = 0x04,
    ATK_DISENCHANT = 0x08,  // Exo attack
    ATK_RAY = 0x10,
    ATK_TP = 0x20,            // Illus attack
    ATK_INVIS = 0x40,        // Illus attack
    ATK_NO_MOVE = 0x80,    // Immobile monster
    ATK_BLOWBACK = 0x100,
    ATK_POLYSELF = 0x200,    // Transmuter attack
    ATK_DESTROY = 0x400,    // Transmuter attack
    ATK_CONJURE = 0x800,    // Evoker attack
    ATK_EVOKE = 0x1000,        // Evoker attack
    ATK_LIFEFORCE = 0x2000, // Necro attack
    ATK_EXPDRAIN = 0x4000,    // Necro attack
    ATK_TIMEOUT = 0x8000,    // Exo attack
    ATK_DROP_SWAG = 0x10000,
} MonAtkType;


typedef enum
{
    MON_RAT,
    MON_SEWER_RAT,
    MON_CHAMELEON,
    MON_HOUND,
    MON_PATRONUS,
    MON_DEATH_HOUND,
    MON_FOX,
    MON_BEAR,
    MON_CAVE_BEAR,
    MON_CRAB,
    MON_HORSESHOE_CRAB,
    MON_TROLL,
    MON_FROST_TROLL,
    MON_GOBLIN,
    MON_HOBGOBLIN,
    MON_FLOATING_EYE,
    MON_LICH,
    MON_ARCH_LICH,
    MON_UNDEAD_GOLEM,
    MON_SHINY_GOLEM,
    MON_CONJURING_GOLEM,
    MON_CHANGING_GOLEM,
    MON_DEMON,
    MON_DEMON_LORD,
    MON_DEMON_KING,
    MON_ANGEL,
    MON_ARCHANGEL,
    MON_LESSER_MINION,
    MON_MINION,
    MON_GREATER_MINION,
    MON_TT,
    MON_ESTEBAN,
    MON_CHARLES,
    MON_ZALE,
    MON_ODYSSEUS,
    MON_GREEN_DRAGON,
    MON_CYAN_DRAGON,
    MON_RED_DRAGON,
    MON_YELLOW_DRAGON,
    MON_BLUE_DRAGON,
    MON_WIZARD_OF_YENDOR,
    MON_SPIRIT,
    MON_HIGHER_SPIRIT,
    MON_GHOST,
    MON_ELF,
    MON_ELF_LORD,
    MON_ELF_KING,
    MON_DWARF,
    MON_DWARF_LORD,
    MON_DWARF_KING,
    MON_ASMODEUS,
    MON_JUIBLEX,
    MON_DEMOGORGON,
    MON_ANT,
    MON_FIRE_ANT,
    MON_JELLY,
} MonTypeTag;

typedef struct
{
    MonTypeTag tag; /* For lookups */

    char *name;
    int max_hp;
    MagicClassTag magic_class;
    char symbol;
    bool bold;
    int prob;
    int difficulty;
    int atk_types;
    int power;
    int ac;
} MonType; /* Monster type */

typedef enum
{
    MF_INVISIBLE,
    MF_ASLEEP,
    MF_FROZEN,
} MonFlags;

typedef struct s_monster
{
    MonType *type;

    int posx, posy, level;
    int hp;
    int timeout; /* How much time before unfreezing/waking/etc. */
    int cooldown;
    int flags; /* Such as invisible, flying ... */
} Monster;

MonType monster_types[MAX_NB_MONSTERS];
int nb_monster_types;


/************/


typedef enum
{
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

typedef struct
{
    int posx, posy; /* Position */
    int explevel; /* Experience stuff */
    int hp, max_hp;
    int dlvl; /* The depth in the dungeon or whatever you may call it */
    int magic_class_exp[NB_MAGIC_CLASSES];

    Object *inventory[INVENTORY_SIZE];
    Object *wielded;
    Object *helm;
    Object *body_armor;
    Mixin_type permanent_effects[MAX_MIXIN];
    int gold;
    int score;
    int ac;
} Player;

/* Prototypes */

void init_ncurses();

void init_game();

void init_colors();

void game_loop();

void exit_game();

void exit_ncurses();

int find_floor_tile(int, int *, int *, int, bool);

void create_level(int);

void recompute_visibility();

char get_input();

void pline(char *, ...);

void clear_msg_line();

bool yes_no(char *, ...);

void display_everything();

void display_map();

void display_stats();

Object *select_object(LinkedList *objects);

void init_monster_types();

void make_monsters(int levels, int nb);

Monster *find_mon_at(int, int, int);

LinkedList *find_objs_at(int, int, int);

int rand_int(int, int);

int ndn(int, int);

int min(int, int);

int max(int, int);

int sign(int);

int abs(int);

double abs_d(double);

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

int rodney_attacks(Monster *target, bool melee);

void move_monsters();

bool check_dead(Monster *target, bool rodney_killed);

bool has_mixin(const ObjectType *type, Mixin_type mixin);

int can_walk(int level, int from_x, int from_y, int to_x, int to_y);

bool
dijkstra(int level, int from_x, int from_y, int to_x, int to_y, int *next_x,
         int *next_y, bool can_have_monst);

void inventory();

int wield();

int unwield();

int wear();

int take_off_armor();

bool is_visible(int level, int from_x, int from_y, int to_x, int to_y,
                bool monsters_block, int *block_x, int *block_y);

void regain_hp();

bool has_inventory_effect(Mixin_type effect);

char *object_name(Object *obj);

int change_dlvl(int to_dlvl, int place_on);

int use();

bool get_point(int *x, int *y, char *format, ...);

Object *select_from_inv(int possible_types);

/* Globals */

typedef enum
{
    TS_UNDISCOVERED,
    TS_UNSEEN,
    TS_SEEN
} TileStatus;

TileType lvl_map[LEVELS][LEVEL_HEIGHT][LEVEL_WIDTH];
TileStatus visibility_map[LEVELS][LEVEL_HEIGHT][LEVEL_WIDTH];
bool visited[LEVELS];
Player rodney;
int turn;

LinkedList *o_list;
LinkedList *m_list;

/* Whether there is something displayed on the status line already */
bool line_needs_confirm;
int last_col;
FILE *log_file;

void print_to_log(char *format, ...);
