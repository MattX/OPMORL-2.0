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

#define MONSTER(tag, mc, mhp, ac, atk, pow, lev, sym, prob) \
                    add_montype(&monster_pointer, tag, mc, mhp, ac, atk, pow, \
                                lev, sym, prob);

/*
 * add_montype: add a monster type on the monster type array, at the position
 * pointed by mon_ptr. Check mon_ptr for bounds and update it.
 */
void add_montype(int *mon_ptr, MonTypeTag tag, MagicClassTag magic_class,
                 int max_hp, int ac, int atk_types, int power, int level,
                 char symbol, int prob)
{
    if (*mon_ptr > MAX_NB_MONSTERS) {
        print_to_log("Tried to register too many monsters\n");
        return;
    }
    monster_types[*mon_ptr].tag = tag;
    monster_types[*mon_ptr].magic_class = magic_class;
    monster_types[*mon_ptr].max_hp = max_hp;
    monster_types[*mon_ptr].ac = ac;
    monster_types[*mon_ptr].atk_types = atk_types;
    monster_types[*mon_ptr].difficulty = level;
    monster_types[*mon_ptr].symbol = symbol;
    monster_types[*mon_ptr].prob = prob;
    (*mon_ptr)++;
}

/*
 * init_monster_types: Register monsters types.
 */
void init_monster_types()
{
    int monster_pointer = 0;

    /* Name, Magic Class, HP, AC, Attacks, Power, Level, Symbol, Probability */
    MONSTER(MON_RAT, MC_EVOK, 6, 6, ATK_MELEE, 2, 1, 'r', 10);

    nb_monster_types = monster_pointer;
}


/*
 * can_spawn_on_level: Checks if a monster mon_id (as a pointer in the monster
 * array, not a monster type flag) can be spawned at the given dlvl with
 * rodney at explevel.
 */
static int can_spawn_on_level(int mon_id, int explevel, int dlvl)
{
    return monster_types[mon_id].difficulty > explevel - 5 &&
           monster_types[mon_id].difficulty <= max(explevel, dlvl);
}

/*
 * choose_montype: Returns a random monster type that can be spawned given
 * rodney's experience and dlvl. The monster is returned as an index in the
 * monster type array. The random selection is weighted by each monster type's
 * probability.
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

/*
 * make_monsters: Adds the specified nb (or 2d6 if nb is non-positive) of
 * monsters to dlvl level.
 */
void make_monsters(int level, int nb)
{
    if (nb <= 0)
        nb = ndn(2, 6);

    for (int i = 0; i < nb; i++) {
        Monster *mon = malloc(sizeof(Monster));
        mon->type = &monster_types[choose_montype(level)];
        mon->flags = 0;
        mon->hp = mon->type->max_hp;
        mon->timeout = 0;
        mon->cooldown = 0;

        mon->level = level;
        if (!find_floor_tile(level, &(mon->posx), &(mon->posy), T_WALKABLE,
                             false)) {
            print_to_log("Level %d full, cannot add monster", level);
            free(mon);
            return;
        }

        add_to_linked_list(m_list, mon);
    }
}

/*
 * find_mon_at: Returns the monster at (level, x, y) if there is one, or NULL.
 */
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
