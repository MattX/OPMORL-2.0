/*
 *  init.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

/** Various initialization and exiting functions */

#include "opmorl.h"

/**
 * Initializes ncurses and logging
 */
void system_init()
{
	initscr();
    cbreak();
	keypad(stdscr, TRUE);
	noecho();
    set_escdelay(25);

    define_colors();
    line_needs_confirm = 0;

    log_file = fopen(LOGFILE_PATH, "a");
    if (log_file == NULL)
        perror("Error opening log file: ");

    fprintf(log_file, "Started OPMORL %s at %d\n", STRING_V, (int) time(NULL));

    line_needs_confirm = false;
    last_col = 0;

    pline("Digging the dungeon, please wait...");
}

#define DECLARE_COLOR_PAIR(id, red, green, blue) init_color(id, red, green, blue); \
init_pair(id, id, DEFAULT_BACKCOLOR)

/**
 * Initializes ncurses colors and pairs
 */
void define_colors()
{
    start_color();

    if (COLORS < 255 || COLOR_PAIRS < 50) {
        printf("OPMORL requires terminal support for 255 colors and 50 pairs");
        exit(0);
    }

    /* Initialize the default pairs. Do not remove these declarations */
    use_default_colors();
    init_pair(CLR_WHITE, COLOR_WHITE, DEFAULT_BACKCOLOR);
    init_pair(CLR_YELLOW, COLOR_YELLOW, DEFAULT_BACKCOLOR);
    init_pair(CLR_MAGENTA, COLOR_MAGENTA, DEFAULT_BACKCOLOR);
    init_pair(CLR_BLUE, COLOR_BLUE, DEFAULT_BACKCOLOR);
    init_pair(CLR_RED, COLOR_RED, DEFAULT_BACKCOLOR);
    init_pair(CLR_CYAN, COLOR_CYAN, DEFAULT_BACKCOLOR);
    init_pair(CLR_GREEN, COLOR_GREEN, DEFAULT_BACKCOLOR);
    init_pair(CLR_BLACK, COLOR_BLACK, DEFAULT_BACKCOLOR);

    /* Custom pairs */
    DECLARE_COLOR_PAIR(CLR_LIGHTGRAY, 700, 700, 700);
    DECLARE_COLOR_PAIR(CLR_DARKGRAY, 200, 200, 200);
    DECLARE_COLOR_PAIR(CLR_LIGHTGREEN, 600, 1000, 600);
    DECLARE_COLOR_PAIR(CLR_DARKGREEN, 200, 500, 200);
    DECLARE_COLOR_PAIR(CLR_STEELBLUE, 445, 559, 804);
}

/**
 * Destructs ncurses handlers
 */
void exit_ncurses() {
    endwin();
}


/**
 * Exits the game
 */
void exit_game() {
#ifdef DEBUG
    bool confirm = yes_no("Are you sure :) ?");
    if (!confirm)
        return;
#endif

    pline("Goodbye.");
    get_input();
    exit_ncurses();
    exit(0);
}

/**
 * Initializes gameplay
 */
void init_game()
{
    srand((unsigned int) time(NULL));

    god_mode = false;

    init_monster_types();
    make_object_classes();
    lever_connections = new_linked_list();

    rodney.gold = ndn(10, 20);
    rodney.explevel = 1;
    rodney.hp = rodney.max_hp = 15;
    rodney.body_armor = rodney.wielded = rodney.helm = NULL;
    rodney.ac = 4;
    rodney.freeze_timeout = 0;

    o_list = new_linked_list();
    m_list = new_linked_list();
    turn = 0;

    for (int i = 0; i < NB_MIXIN; i++) {
        rodney.permanent_effects[i] = MT_NONE;
    }
    for (int i = 0; i < NB_MAGIC_CLASSES; i++) {
        rodney.magic_class_exp[i] = 0;
    }
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        rodney.inventory[i] = NULL;
    }
    Object *rodneys_basic_sword = malloc(sizeof(Object));
    rodneys_basic_sword->type = &object_types[1];
    rodneys_basic_sword->enchant = 0;
    rodneys_basic_sword->flags = 0;
    rodneys_basic_sword->cooldown = 0;
    rodneys_basic_sword->uses_left = -1;

    rodney.inventory[0] = rodneys_basic_sword;
    rodney.wielded = rodneys_basic_sword;

    layout_dungeon();
    log_layout();

    load_grid();
    for (int i_dlvl = 0; i_dlvl < DLVL_MAX; i_dlvl++) {
        generate_level(i_dlvl);
        add_level_objects(i_dlvl);
    }

    rodney.dlvl = 0;
    change_dlvl(0, T_STAIRS_UP, true);

    pline("Welcome to OPMORL!");
    show_intro();

    recompute_visibility();
}