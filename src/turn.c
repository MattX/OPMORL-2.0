//
// Created by Matthieu Felix on 18/09/2016.
//

#include "opmorl.h"
#include "linkedlist.h"

int process_move_input(char c)
{
    int to_x = rodney.posx;
    int to_y = rodney.posy;
    Monster *target;

    if (c == 'h' || c == 'y' || c == 'b')
        to_y--;
    else if (c == 'l' || c == 'u' || c == 'n')
        to_y++;

    if (c == 'j' || c == 'b' || c == 'n')
        to_x++;
    else if (c == 'k' || c == 'y' || c == 'u')
        to_x--;

    if ((target = find_mon_at(rodney.dlvl, to_x, to_y)) != NULL) {
        if (target->flags & MF_INVISIBLE) {
            pline("Wait! There's a %s there!", target->type->name);
            target->flags &= ~MF_INVISIBLE;
            return 1;
        } else {
            return rodney_attacks(target, false);
        }
    }

    return move_rodney(to_x, to_y);
}


void show_env_messages()
{
    LinkedList *objs_on_tile = find_objs_at(rodney.posx, rodney.posy, rodney.dlvl);

    if (objs_on_tile->length == 1)
        pline("You see here a %s.",
              object_name((Object *) objs_on_tile->head->element));
    else if (objs_on_tile->length > 1)
        pline("You see here a %s and other objects.",
              object_name((Object *) objs_on_tile->head->element));
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
    case 'q':
        exit_game();
        break;
    default:
        break;
    }

    if (turn_elapsed) {
        move_monsters();
        regain_hp();
        recompute_visibility();
        turn++;
    }
}

