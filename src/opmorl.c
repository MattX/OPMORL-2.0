/*
 *  opmorl.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"

int main(void)
{
    printf("OPMORL %s", STRING_V);

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