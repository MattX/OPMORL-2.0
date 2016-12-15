/*
 *  init.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

/* Various initialization functions */

#include "opmorl.h"

void init_ncurses() {
	initscr();
    cbreak();
	keypad(stdscr, TRUE);
	noecho();

	init_colors();
    line_needs_confirm = 0;

    log_file = fopen(LOGFILE_PATH, "a");
    if (log_file == NULL)
        perror("Error opening log file: ");

    fprintf(log_file, "Started OPMORL %s at %d\n", STRING_V, (int) time(NULL));

    line_needs_confirm = false;
    last_col = 0;

    pline("Digging the dungeon, please wait...");
}

void init_colors() {
	start_color();
	use_default_colors();
	init_pair(CLR_WHITE, COLOR_WHITE, DEFAULT_BACKCOLOR); /* broken by the enum, wtf */
    init_pair(CLR_YELLOW, COLOR_YELLOW, DEFAULT_BACKCOLOR);
    init_pair(CLR_MAGENTA, COLOR_MAGENTA, DEFAULT_BACKCOLOR);
    init_pair(CLR_BLUE, COLOR_BLUE, DEFAULT_BACKCOLOR);
    init_pair(CLR_RED, COLOR_RED, DEFAULT_BACKCOLOR);
    init_pair(CLR_CYAN, COLOR_CYAN, DEFAULT_BACKCOLOR);
    init_pair(CLR_GREEN, COLOR_GREEN, DEFAULT_BACKCOLOR);
}

void exit_ncurses() {
	endwin();
}


void exit_game() {
    bool confirm = yes_no("Are you sure :) ?");
    if (!confirm)
        return;

    pline("Goodbye.");
	getch();
    exit_ncurses();
    exit(0);
}

void init_game()
{
    srand((unsigned int) time(NULL));

    god_mode = false;

    init_monster_types();
    make_object_classes();

    rodney.gold = ndn(10, 20);
    rodney.explevel = 1;
    rodney.hp = rodney.max_hp = 15;
    rodney.body_armor = rodney.wielded = rodney.helm = NULL;
    rodney.ac = 4;

    o_list = new_linked_list();
    m_list = new_linked_list();
    turn = 0;

    for (int i = 0; i < MAX_MIXIN; i++) {
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

    load_grid();
    for (int i_dlvl = 0; i_dlvl < DLVL_MAX; i_dlvl++) {
        make_layout_from_grid(i_dlvl);
        add_level_objects(i_dlvl);
    }

    rodney.dlvl = 0;
    change_dlvl_stairs(0, T_STAIRS_UP);

    pline("Welcome to OPMORL!");
    show_intro();

    recompute_visibility();
}