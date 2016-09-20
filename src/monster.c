/*
 *  monster.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

/* Same remarks apply for monsters that the ones in object.c 
 * No way. You dont need the player to hold monsters thus you only need 
 */

#include "opmorl.h"

#define MONSTER(tag, mc, mhp, lev, sym, prob) add_montype(&monster_pointer, tag, mc, mhp, lev, sym, prob);

void add_montype(int *mon_ptr, MonTypeTag tag, MagicClassTag magic_class, int max_hp, int level, char symbol, int prob)
{
    if (*mon_ptr > MAX_NB_MONSTERS) {
        print_to_log("Tried to register too many monsters\n");
        return;
    }
    monster_types[*mon_ptr].tag = tag;
    monster_types[*mon_ptr].magic_class = magic_class;
    monster_types[*mon_ptr].max_hp = max_hp;
    monster_types[*mon_ptr].level = level;
    monster_types[*mon_ptr].symbol = symbol;
    monster_types[*mon_ptr].prob = prob;
}

void init_monsters()
{
    int monster_pointer = 0;

    MONSTER(MON_RAT, MC_EVOK, 6, 1, 'r', 10);

    nb_monster_types = monster_pointer;
}


static int can_spawn_on_level(int mon_id, int explevel, int dlvl)
{
    return monster_types[mon_id].level > explevel - 5 &&
           monster_types[mon_id].level <= max(explevel, dlvl);
}

/*
 * Choose an index from the monstertype array, weigthing by prob amongst suitable monsters
 */
int choose_montype(int level)
{
    int total_prob = 0;

    for (int i = 0; i < nb_monster_types; i++)
        if (can_spawn_on_level(i, rodney.explevel, rodney.dlvl))
            total_prob += monster_types[i].prob;

    if (total_prob == 0) {
        print_to_log("No suitable monsters found\n");
        return 0;
    }

    int prob_index = rand_int(1, total_prob);
    int prob_so_far = 0;
    int i = 0;
    for (i = 0; i < nb_monster_types && prob_so_far < prob_index; i++)
        if (can_spawn_on_level(i, rodney.explevel, rodney.dlvl))
            prob_so_far += monster_types[i].prob;

    return i;
}

void make_monsters(int level, int nb)
{
    if (nb == -1)
        nb = ndn(2, 5);

    for (int i = 0; i < nb; i++) {
        Monster *mon = malloc(sizeof(Monster));
        mon->type = &monster_types[choose_montype(level)];
        mon->flags = 0;
        mon->hp = mon->type->max_hp;
        mon->move_timeout = 0;

        mon->level = level;
        if (!find_floor_tile(level, &(mon->posx), &(mon->posy), T_WALKABLE, false)) {
            print_to_log("Level %d full, cannot add monster", level);
            free(mon);
            return;
        }

        add_to_linked_list(m_list, mon);
        pline("Added monster at %d, %d\n", mon->posx, mon->posy);
    }
}

Monster *find_mon_at(int x, int y, int level)
{
    LinkedListNode *mon_node = m_list->head;

    if (!mon_node)
        return NULL; /* Be careful with that one ! */

    while (mon_node != NULL) {
        Monster *mon = (Monster *) mon_node->element;
        if (mon->posx == x && mon->posy == y && mon->level == level)
            return mon;

        mon_node = mon_node->next;
    }

    return NULL;
}
