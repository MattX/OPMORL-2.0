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

#define MONSTER(tag, nam, mc, mhp, ac, atk, pow, lev, sym, bold, prob) \
                    add_montype(&monster_pointer, tag, nam, mc, mhp, ac, atk, \
                                pow, lev, sym, bold, prob);

/*
 * add_montype: add a monster type on the monster type array, at the position
 * pointed by mon_ptr. Check mon_ptr for bounds and update it.
 */
void add_montype(int *mon_ptr, MonTypeTag tag, char *name,
                 MagicClassTag magic_class, int max_hp, int ac, int atk_types,
                 int power, int level, char symbol, bool bold, int prob)
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
    monster_types[*mon_ptr].power = power;
    monster_types[*mon_ptr].name = name;
    monster_types[*mon_ptr].bold = bold;
    (*mon_ptr)++;
}

/*
 * init_monster_types: Register monsters types.
 */
void init_monster_types()
{
    int monster_pointer = 0;

    /* Name, Magic Class, HP, AC, Attacks, Power, Level, Symbol, Probability */
    MONSTER(MON_RAT, "rat", MC_NEUTRAL, 6, 5, ATK_MELEE, 2, 1, 'r', false, 80);
    MONSTER(MON_SEWER_RAT, "sewer rat", MC_NEUTRAL, 7, 5, ATK_MELEE, 3, 1, 'r',
            true, 80);
    MONSTER(MON_CHAMELEON, "chameleon", MC_ILLU, 10, 6, ATK_MELEE | ATK_INVIS,
            0, 3, 'c', false, 20);
    MONSTER(MON_HOUND, "hound", MC_EXOR, 15, 4, ATK_MELEE, 4, 2, 'h', false,
            80);
    MONSTER(MON_DEATH_HOUND, "death hound", MC_NECRO, 15, 4, ATK_MELEE, 4, 2,
            'h', false, 80);
    MONSTER(MON_PATRONUS, "patronus", MC_TRANS, 15, 4, ATK_MELEE | ATK_POLYSELF,
            4, 5, 'h', false, 20);
    MONSTER(MON_FOX, "fox", MC_NEUTRAL, 15, 4, ATK_MELEE, 4, 2, 'f', false, 80);
    MONSTER(MON_BEAR, "bear", MC_NEUTRAL, 35, 3, ATK_MELEE, 6, 4, 'b', false,
            80);
    MONSTER(MON_CAVE_BEAR, "cave bear", MC_NEUTRAL, 40, 3, ATK_MELEE, 8, 5, 'b',
            true, 40);
    MONSTER(MON_CRAB, "crab", MC_NEUTRAL, 15, 3, ATK_MELEE, 3, 2, 'c', true,
            80);
    MONSTER(MON_HORSESHOE_CRAB, "horseshoe crab", MC_NEUTRAL, 5, -1, ATK_MELEE,
            1, 1, 'c', false, 80);
    MONSTER(MON_TROLL, "troll", MC_NEUTRAL, 50, 1, ATK_MELEE, 12, 8, 'T', false,
            80);
    MONSTER(MON_FROST_TROLL, "frost troll", MC_NEUTRAL, 40, 1,
            ATK_MELEE | ATK_FREEZE, 10, 8, 'T', true, 40);
    MONSTER(MON_GOBLIN, "goblin", MC_NEUTRAL, 18, 3, ATK_MELEE, 4, 3, 'g',
            false, 80);
    MONSTER(MON_HOBGOBLIN, "hobgoblin", MC_NEUTRAL, 25, 3, ATK_MELEE, 4, 3, 'g',
            true, 80);
    MONSTER(MON_FLOATING_EYE, "floating eye", MC_TRANS, 15, 3, 0, 0, 5, 'e',
            false, 20);
    MONSTER(MON_LICH, "lich", MC_NECRO, 40, 0, ATK_MELEE | ATK_RAY, 7, 7, 'L',
            false, 80);
    MONSTER(MON_ARCH_LICH, "fucking arch-lich", MC_NECRO, 60, -1,
            ATK_MELEE | ATK_RAY | ATK_LIFEFORCE | ATK_EXPDRAIN, 10, 15, 'L',
            true, 2);
    MONSTER(MON_UNDEAD_GOLEM, "undead golem", MC_NECRO, 30, 2, ATK_MELEE, 6, 5,
            'G', false, 80);
    MONSTER(MON_SHINY_GOLEM, "shiny golem", MC_ILLU, 30, 2, ATK_MELEE, 6, 5,
            'G', false, 80);
    MONSTER(MON_CONJURING_GOLEM, "conjuring golem", MC_EXOR, 30, 2, ATK_MELEE,
            6, 5,
            'G', false, 80);
    MONSTER(MON_CHANGING_GOLEM, "changing golem", MC_TRANS, 30, 2, ATK_MELEE, 6,
            5,
            'G', false, 80);
    MONSTER(MON_DEMON, "demon", MC_NECRO, 20, 4, ATK_MELEE, 4, 2, 'd', false,
            80);
    MONSTER(MON_DEMON_LORD, "demon lord", MC_NECRO, 30, 2, ATK_MELEE, 7, 5, 'D',
            false, 80);
    MONSTER(MON_DEMON_KING, "demon king", MC_NECRO, 50, 0,
            ATK_MELEE | ATK_LIFEFORCE, 9, 11, 'D', true, 80);
    MONSTER(MON_ANGEL, "anglel", MC_EXOR, 30, 0, ATK_MELEE | ATK_TP, 6, 7, 'A',
            false, 20);
    MONSTER(MON_ARCHANGEL, "archangel", MC_EXOR, 35, -1,
            ATK_MELEE | ATK_TP | ATK_TIMEOUT, 10, 13, 'A', true, 20);
    MONSTER(MON_LESSER_MINION, "lesser minion", MC_TRANS, 20, 4, ATK_MELEE, 4,
            2, 'm', false, 80);
    MONSTER(MON_MINION, "minion", MC_NECRO, 30, 2, ATK_MELEE, 7, 5, 'M', false,
            80);
    MONSTER(MON_GREATER_MINION, "greater minion", MC_NECRO, 50, 0,
            ATK_MELEE | ATK_LIFEFORCE, 9, 11, 'M', true, 80);
    MONSTER(MON_TT, "tt", MC_NECRO, 75, -6,
            ATK_MELEE | ATK_RAY | ATK_LIFEFORCE | ATK_EXPDRAIN, 15, 25, '@',
            true, 0);
    MONSTER(MON_ESTEBAN, "esteban", MC_ILLU, 75, -6,
            ATK_MELEE | ATK_RAY | ATK_TP | ATK_INVIS, 15, 25, '@', true, 0);
    MONSTER(MON_CHARLES, "charles", MC_EVOK, 75, -6,
            ATK_MELEE | ATK_RAY | ATK_EVOKE | ATK_CONJURE, 15, 25, '@', true,
            0);
    MONSTER(MON_ZALE, "zale", MC_EXOR, 75, -6,
            ATK_MELEE | ATK_RAY | ATK_DISENCHANT | ATK_TIMEOUT, 15, 25, '@',
            true, 0);
    MONSTER(MON_ODYSSEUS, "odysseus", MC_TRANS, 75, -6,
            ATK_MELEE | ATK_RAY | ATK_DESTROY | ATK_POLYSELF, 15, 25, '@', true,
            0);
    MONSTER(MON_GREEN_DRAGON, "green dragon", MC_EVOK, 50, -2,
            ATK_MELEE | ATK_RAY | ATK_CONJURE, 15, 13, 'D', true, 0);
    MONSTER(MON_CYAN_DRAGON, "cyan dragon", MC_NECRO, 50, -2,
            ATK_MELEE | ATK_RAY | ATK_EXPDRAIN, 15, 13, 'D', true, 0);
    MONSTER(MON_RED_DRAGON, "red dragon", MC_TRANS, 50, -2,
            ATK_MELEE | ATK_RAY | ATK_POLYSELF, 15, 25, 'D', true,
            0);
    MONSTER(MON_YELLOW_DRAGON, "yellow dragon", MC_ILLU, 50, -2,
            ATK_MELEE | ATK_RAY | ATK_INVIS, 15, 25, 'D',
            true, 0);
    MONSTER(MON_BLUE_DRAGON, "blue dragon", MC_EXOR, 50, -2,
            ATK_MELEE | ATK_RAY | ATK_TIMEOUT, 15, 25, 'D', true,
            0);


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
    for (i = 0; i < nb_monster_types; i++)
        if (can_spawn_on_level(i, rodney.explevel, rodney.dlvl)) {
            prob_so_far += monster_types[i].prob;
            if (prob_so_far >= prob_index)
                break;
        }

    pline("Choosing %s, difficulty %d", monster_types[i].name,
          monster_types[i].difficulty);

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
