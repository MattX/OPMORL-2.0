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
 */
void process_turn()
{
    int turns_elapsed = 0;
    char c;

    if (rodney.freeze_timeout > 0) {
        c = '.'; // Force wait
    } else {
        c = get_input();
        clear_msg_line();
    }

    switch (c) {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case 'y':
    case 'u':
    case 'b':
    case 'n':
        turns_elapsed = process_move_input(c);
        break;
    case '>':
        turns_elapsed = use_stairs(0);
        break;
    case '<':
        turns_elapsed = use_stairs(1);
        break;
    case ',':
        turns_elapsed = pickup();
        break;
    case 'd':
        turns_elapsed = drop();
        break;
    case 'i':
        inventory();
        break;
    case 'w':
        turns_elapsed = wield();
        break;
    case 'x':
        turns_elapsed = unwield();
        break;
    case 'W':
        turns_elapsed = wear();
        break;
    case 'T':
        turns_elapsed = take_off_armor();
        break;
    case '.':
        turns_elapsed = 1;
        break;
    case 'a':
        turns_elapsed = use();
        break;
    case 'z':
        turns_elapsed = zap();
        break;
    case 'o':
        turns_elapsed = toggle_door((Coord) {0, 0}, true);
        break;
    case 'c':
        turns_elapsed = toggle_door((Coord) {0, 0}, false);
        break;
    case 't':
        turns_elapsed = toggle_lever();
        break;
    case 'E':
        turns_elapsed = teleport();
        break;
#ifdef DEBUG
    case 'L':
        display_layout();
        break;
    case 'G':
        god_mode = !god_mode;
        pline("God mode %s.", god_mode ? "enabled" : "disabled");
        break;
    case 'S':
        test_multiple_selection();
        break;
    case 'F':
        rodney.freeze_timeout = 10;
        break;
#endif
    case 'R':
        redrawwin(stdscr);
        break;
    case 'q':
        exit_game();
        break;
    case 0:
    default:
        break;
    }

    while (turns_elapsed--) {
        tick_monsters();
        regain_hp();
        recompute_visibility();
        turn++;
    }
}
