//
// Created by Matthieu Felix on 18/09/2016.
//

#include "opmorl.h"

int process_move_input(char c)
{
    int to_x = rodney.posx;
    int to_y = rodney.posy;

    if (c == 'h' || c == 'y' || c == 'b')
        to_y--;
    else if (c == 'l' || c == 'u' || c == 'n')
        to_y++;

    if (c == 'j' || c == 'b' || c == 'n')
        to_x++;
    else if (c == 'k' || c == 'y' || c == 'u')
        to_x--;

    return move_rodney(to_x, to_y);
}


void process_turn(char c)
{
    switch (c) {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case 'y':
    case 'u':
    case 'b':
    case 'n':
        process_move_input(c);
        break;
    case '>':
        use_stairs(0);
        break;
    case '<':
        use_stairs(1);
        break;
    case 'q': /* Need to rewrite it cleanly */
        exit_game();
        exit_ncurses();
        exit(0);
    default:
        break;
    }
}

