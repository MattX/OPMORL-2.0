/*
 *  turn.c
 *  OPMORL 2
 *
 *  Created by Matthieu Felix on 18/09/2016.
 *  Copyright 2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"

/**
 * Processes a movement key: either movement or attack, depending on the
 * situation
 * @param c The movement key
 * @return The number of turns the action took to complete
 */
int process_move_input(char c)
{
    Coord to = rodney.pos;

    to = coord_add(to, letter_to_direction(c));

    return move_rodney(to);
}

/**
 * Shows the relevant environment messages
 */
void show_env_messages()
{
    LinkedList *objs_on_tile = find_objs_at(rodney.dlvl, rodney.pos);

    if (objs_on_tile->length == 1)
        pline("You see here a %s.",
              object_name((Object *) objs_on_tile->head->element));
    else if (objs_on_tile->length > 1)
        pline("You see here a %s and other objects.",
              object_name((Object *) objs_on_tile->head->element));
}


/**
 * Processes one (or more) full turns
 * @param c The command the player entered
 */
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
    case ',':
        turn_elapsed = pickup();
        break;
    case 'd':
        turn_elapsed = drop();
        break;
    case 'i':
        inventory();
        break;
    case 'w':
        turn_elapsed = wield();
        break;
    case 'x':
        turn_elapsed = unwield();
        break;
    case 'W':
        turn_elapsed = wear();
        break;
    case 'T':
        turn_elapsed = take_off_armor();
        break;
    case '.':
        turn_elapsed = true;
        break;
    case 'a':
        turn_elapsed = use();
        break;
    case 'z':
        turn_elapsed = zap();
        break;
    case 'o':
        turn_elapsed = toggle_door((Coord) {0, 0}, true);
        break;
    case 'c':
        turn_elapsed = toggle_door((Coord) {0, 0}, false);
        break;
    case 't':
        turn_elapsed = toggle_lever();
        break;
#ifdef DEBUG
    case 'G':
        god_mode = !god_mode;
        pline("God mode %s.", god_mode ? "enabled" : "disabled");
        break;
#endif
    case 'R':
        redrawwin(stdscr);
        break;
    case 'q':
        exit_game();
        break;
    default:
        break;
    }

    while (turn_elapsed--) {
        move_monsters();
        regain_hp();
        recompute_visibility();
        turn++;
    }
}

