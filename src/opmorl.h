/*
 *  opmorl.h
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
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
#include <curses.h>

#include "linkedlist.h"

#define DEBUG

#define VERSION 0.01
#define STRING_V "dev"
#define DEFAULT 0
#define DEFAULT_BACKCOLOR -1
#define DEFAULT_FORECOLOR -1

#define DLVL_MAX 16
#define LEVEL_WIDTH 80
#define LEVEL_HEIGHT 20

#define INVENTORY_SIZE 52
// Different object types
#define NB_OBJECTS 70

#define MAX_NAME 50
#define MAX_DESCR 200

#define LOGFILE_PATH "opmorl.log"
#define INTRO_FILE "dat/intro.txt"
#define GRID_FILE "dat/grid.txt"

/* Structs */


typedef enum e_color
{
    CLR_DEFAULT, /* COLOR_PAIR(0) is the default back/fore ground colors */
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


/**
 * Stores a coordinate
 */
typedef struct s_coord
{
    int x; /** x coordinate */
    int y; /** y coordinate */
} Coord;

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
    OT_MONEY = 1 << 1,
    OT_MELEE = 1 << 2,
    OT_POTION = 1 << 3,
    OT_WAND = 1 << 4,
    OT_TOOL = 1 << 5,
    OT_BODY_ARMOR = 1 << 6,
    OT_HELM = 1 << 7,
    OT_FOOD = 1 << 8,
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
    MT_NUM,
    MT_NONE,
} MixinType; // To be able to quickly check a mixin

typedef struct
{
    MixinType id;
    int compatible_classes;
    char *descr;
    int prob;
    bool util;
} Mixin;


typedef struct s_object
{
    const ObjectType *type;

    Coord pos;
    int dlvl; /* Coordinates : dlvl (we want persistent levels) */
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
    ATK_MELEE = 1 << 1,
    ATK_FREEZE = 1 << 2,
    ATK_FIRE = 1 << 3,
    ATK_DISENCHANT = 1 << 4,
    ATK_RAY = 1 << 5,
    ATK_TP = 1 << 6,
    ATK_INVIS = 1 << 7,
    ATK_NO_MOVE = 1 << 8,    // Immobile monster
    ATK_BLOWBACK = 1 << 9,
    ATK_POLYSELF = 1 << 10,
    ATK_DESTROY = 1 << 11,
    ATK_CONJURE = 1 << 12,
    ATK_EVOKE = 1 << 13,
    ATK_LIFEFORCE = 1 << 14,
    ATK_EXPDRAIN = 1 << 15,
    ATK_TIMEOUT = 1 << 16,
    ATK_DROP_SWAG = 1 << 17,
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
    MON_NB,
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

    Coord pos;
    int dlvl;
    int hp;
    int timeout; /** How much time before unfreezing/waking/etc. */
    int cooldown;
    int flags; /** Such as invisible, flying ... */
    Coord remembered_pos; /** Where the monster thinks the player is */
    bool remembered;
} Monster;

MonType monster_types[MAX_NB_MONSTERS];
int nb_monster_types;


/************/


typedef enum
{
    T_WALL,
    T_CORRIDOR,
    T_OPEN_DOOR,
    T_CLOSED_DOOR,
    T_FLOOR,
    T_STAIRS_UP,
    T_STAIRS_DOWN,
    T_GROUND,
    T_COLLAPSED,
    T_TRAPDOOR_CLOSED,
    T_TRAPDOOR_OPEN,
    T_LEVER,
    T_PIPE,
    T_PIPE_EXHAUST,
    T_GRASS,
    T_FUNGUS,
    T_TREE,
    T_FOUNTAIN,
    T_PORTCULLIS_UP,
    T_PORTCULLIS_DOWN,
    T_RUBBLE,
    NB_TILE_TYPES
} TileType;

struct s_tile_type
{
    bool walkable;
    char sym;
    Color color;
};

#define IS_WALKABLE(x) (tile_types[x].walkable)

extern struct s_tile_type tile_types[NB_TILE_TYPES];


/***********/

typedef struct
{
    Coord pos; /* Position */
    int explevel; /* Experience stuff */
    int hp, max_hp;
    int dlvl; /* The depth in the dungeon or whatever you may call it */
    int magic_class_exp[NB_MAGIC_CLASSES];

    Object *inventory[INVENTORY_SIZE];
    Object *wielded;
    Object *helm;
    Object *body_armor;
    MixinType permanent_effects[MAX_MIXIN];
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

int find_tile(int, Coord *coords, bool, int);

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

void make_monsters(int dlvl, int nb);

Monster *find_mon_at(int, Coord);

LinkedList *find_objs_at(int, Coord);

int rand_int(int, int);

int ndn(int, int);

int min(int, int);

int max(int, int);

int sign(int);

int abs(int);

double abs_d(double);

void strncpy_pad(char *dest, const char *src, size_t n);

int move_rodney(Coord to);

int use_stairs(bool);

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

bool has_mixin(const ObjectType *type, MixinType mixin);

bool can_walk(int, Coord, Coord);

bool dijkstra(int dlvl, Coord to, Coord from, Coord *next,
              bool can_have_monst);

void inventory();

int wield();

int unwield();

int wear();

int take_off_armor();

bool
is_visible(int dlvl, Coord from, Coord to, Coord *block, bool monsters_block);

void regain_hp();

bool has_inventory_effect(MixinType effect);

char *object_name(Object *obj);

int change_dlvl_stairs(int, int);

int use();

bool get_point(Coord *selected);

Object *select_from_inv(int possible_types);

int zap();

void show_intro();

bool load_grid();

void make_layout_from_grid(int dlvl);

Coord letter_to_direction(char);

Coord get_direction();

Coord coord_add(Coord, Coord);

int open();

/* Globals */

typedef enum
{
    TS_UNDISCOVERED,
    TS_UNSEEN,
    TS_SEEN
} TileStatus;


TileType maps[DLVL_MAX][LEVEL_HEIGHT][LEVEL_WIDTH];
TileStatus visibility_map[DLVL_MAX][LEVEL_HEIGHT][LEVEL_WIDTH];
bool visited[DLVL_MAX];
Player rodney;
int turn;

LinkedList *o_list;
LinkedList *m_list;

/* Whether there is something displayed on the status line already */
bool line_needs_confirm;
int last_col;
FILE *log_file;

bool god_mode;

void print_to_log(char *format, ...);
