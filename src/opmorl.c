/*
 *  opmorl.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"

/* if MCS[human][orc] = true, a human attacking an orc will have an advantage */
bool magic_class_strengths[NB_MAGIC_CLASSES][NB_MAGIC_CLASSES] = {
        /*  Evok    Conj    Necro   Illu    Trans  (<-atk, def v) */
        {false, false, false, false, false},        /* Evok */
        {false, false, true,  false, true},        /* Conj */
        {false, false, true,  true,  false},        /* Necro */
        {false, true,  false, false, false},        /* Illu */
        {false, true,  false, false, false}         /* Trans */
};
char *magic_class_names[NB_MAGIC_CLASSES] = {"evoker", "conjurer", "necromancer", "illusionist", "transmuter"};
char *magic_class_adjectives[NB_MAGIC_CLASSES] = {"evoking", "conjuring", "necromantic", "illusive", "transmuting"};
Color magic_class_colors[NB_MAGIC_CLASSES] = {CLR_GREEN, CLR_MAGENTA, CLR_RED, CLR_YELLOW, CLR_CYAN};


int main(void)
{
    init_ncurses();
    init_game();
    display_everything();
    game_loop();
    exit_game();
    exit_ncurses();
    return 0;
}

void game_loop()
{
    while (1) {
        line_displayed = 0;
        process_turn(get_input());
        display_everything();
    }
}