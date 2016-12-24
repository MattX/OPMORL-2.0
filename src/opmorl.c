/*
 *  opmorl.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"

/* if MCS[human][orc] = true, a human attacking an orc will have an advantage */
bool magic_class_strengths[NB_MAGIC_CLASSES][NB_MAGIC_CLASSES] = {
        /* Neu  Evok   Necro  Pallad  (<-atk, def v) */
        {false, false, false, false},     /* Neu */
        {false, false, false, true},      /* Evok */
        {false, false, true,  false},     /* Conj */
        {false, true,  false, false},     /* Necro */
};
char *magic_class_adjectives[NB_MAGIC_CLASSES] = {"neutral", "evoking",
                                                  "necromantic", "palladin"};
char *magic_class_names[NB_MAGIC_CLASSES] = {"magician", "evoker",
                                             "necromancer", "palladin"};
Color magic_class_colors[NB_MAGIC_CLASSES] = {CLR_WHITE, CLR_GREEN, CLR_MAGENTA,
                                              CLR_CYAN};


int main(void)
{
    system_init();
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
        process_turn(get_input());
        display_everything();
    }
}