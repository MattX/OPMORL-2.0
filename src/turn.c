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


void show_env_messages()
{
    Object *o;
    if ((o = find_obj_at(rodney.posx, rodney.posy, rodney.dlvl)) != NULL)
        pline("You see here a %s", o->type->name);
}

void process_turn(char c)
{
    int turn_elapsed = 0;

    switch (c) {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case 'y':
    case 'u':
    case 'b':
    case 'n':
        turn_elapsed = process_move_input(c);
        break;
    case '>':
        turn_elapsed = use_stairs(0);
        break;
    case '<':
        turn_elapsed = use_stairs(1);
        break;
    case 'q':
        exit_game();
        break;
    default:
        break;
    }

    show_env_messages();
}

