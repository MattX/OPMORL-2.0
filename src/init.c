/*
 *  init.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
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
    line_displayed = 0;

    log_file = fopen(LOGFILE_PATH, "a");
    if (log_file == NULL)
        perror("Error opening log file: ");

    fprintf(log_file, "Started OPMORL %s at %d\n", STRING_V, (int) time(NULL));

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
}

void exit_ncurses() {
	endwin();
}


void exit_game() {
    pline("Goodbye.");
	getch();
    exit_ncurses();
    exit(0);
}

void init_game()
{
    srand((unsigned int) time(NULL));

    make_objects();

    rodney.gold = rodney.exp = 0;
    rodney.explevel = 1;
    rodney.hp = rodney.max_hp = rand_int(12, 18);
    rodney.charisma = rand_int(12, 18);
    rodney.constitution = rand_int(12, 18);
    rodney.strength = rand_int(12, 18);
    rodney.dexterity = rand_int(12, 18);
    rodney.wisdom = rand_int(12, 18);
    rodney.body_armor = rodney.wielded = rodney.helm = NULL;
    rodney.magic_class = rand_int(0, NB_MAGIC_CLASSES - 1);

    rodney.color = CLR_WHITE;

    o_list = new_linked_list();
    m_list = new_linked_list();
    turn = 0;

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        rodney.inventory[i] = NULL;
    }
    rodney.gold = 100;


    line_displayed = 0;
    last_col = 0;

    for (int i_level = 0; i_level < LEVELS; i_level++)
        create_level(i_level);

    rodney.dlvl = 0;
    if (!find_floor_tile(rodney.dlvl, &rodney.posx, &rodney.posy, T_STAIRS_UP, 0)) {
        pline("Panic: Could not add Rodney!");
        exit_game();
    }

    recompute_visibility();

    pline("Welcome to OPMORL !");
    pline("You are a novice %s on your final training.", magic_class_names[rodney.magic_class]);
    pline("Your goal is to escape this dungeon with as much gold as you can.");
}