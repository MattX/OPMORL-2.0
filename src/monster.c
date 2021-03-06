/*
 *  monster.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 20/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
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
    monster_types[*mon_ptr].flags = atk_types;
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
    MONSTER(MON_RAT, "rat", MC_NEUTRAL, 3, 5, MTFLAG_ATK_MELEE, 3, 1, 'r',
            false, 80);
    MONSTER(MON_SEWER_RAT, "sewer rat", MC_NEUTRAL, 4, 5, MTFLAG_ATK_MELEE, 3,
            1, 'r',
            true, 80);
    MONSTER(MON_CHAMELEON, "chameleon", MC_EVOK, 10, 6,
            MTFLAG_ATK_MELEE | MTFLAG_INVIS,
            0, 3, 'c', false, 20);
    MONSTER(MON_HOUND, "hound", MC_PALLADIN, 15, 4, MTFLAG_ATK_MELEE, 4, 2, 'h',
            false,
            80);
    MONSTER(MON_DEATH_HOUND, "death hound", MC_NECRO, 15, 4, MTFLAG_ATK_MELEE,
            4, 2,
            'h', false, 80);
    MONSTER(MON_PATRONUS, "patronus", MC_EVOK, 15, 4,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_POLYSELF,
            4, 5, 'h', false, 20);
    MONSTER(MON_FOX, "fox", MC_NEUTRAL, 15, 4, MTFLAG_ATK_MELEE, 4, 2, 'f',
            false, 80);
    MONSTER(MON_BEAR, "panserbjørn", MC_PALLADIN, 35, 3, MTFLAG_ATK_MELEE, 6, 4,
            'b', false,
            80);
    MONSTER(MON_CAVE_BEAR, "cave bear", MC_PALLADIN, 40, 3, MTFLAG_ATK_MELEE, 8,
            5, 'b',
            true, 40);
    MONSTER(MON_CRAB, "crab", MC_NEUTRAL, 15, 3, MTFLAG_ATK_MELEE, 3, 2, 'c',
            true,
            80);
    MONSTER(MON_HORSESHOE_CRAB, "horseshoe crab", MC_NEUTRAL, 5, -1,
            MTFLAG_ATK_MELEE,
            1, 1, 'c', false, 80);
    MONSTER(MON_TROLL, "troll", MC_NEUTRAL, 50, 1, MTFLAG_ATK_MELEE, 12, 8, 'T',
            false,
            80);
    MONSTER(MON_FROST_TROLL, "frost troll", MC_NEUTRAL, 40, 1,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_FREEZE, 10, 8, 'T', true, 40);
    MONSTER(MON_GOBLIN, "goblin", MC_NEUTRAL, 18, 3, MTFLAG_ATK_MELEE, 4, 3,
            'g',
            false, 80);
    MONSTER(MON_HOBGOBLIN, "hobgoblin", MC_NEUTRAL, 25, 3, MTFLAG_ATK_MELEE, 4,
            3, 'g',
            true, 80);
    MONSTER(MON_FLOATING_EYE, "floating eye", MC_EVOK, 15, 3, 0, 0, 5, 'e',
            true, 20);
    MONSTER(MON_LICH, "lich", MC_NECRO, 40, 0,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY, 7, 7, 'L',
            false, 80);
    MONSTER(MON_ARCH_LICH, "fucking arch-lich", MC_NECRO, 60, -1,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_SPELL |
            MTFLAG_ATK_EXPDRAIN, 10, 15, 'L',
            true, 2);
    MONSTER(MON_UNDEAD_GOLEM, "undead golem", MC_NECRO, 30, 2, MTFLAG_ATK_MELEE,
            6, 5,
            'G', false, 80);
    MONSTER(MON_SHINY_GOLEM, "shiny golem", MC_EVOK, 30, 2, MTFLAG_ATK_MELEE, 6,
            5,
            'G', false, 80);
    MONSTER(MON_CONJURING_GOLEM, "conjuring golem", MC_PALLADIN, 30, 2,
            MTFLAG_ATK_MELEE,
            6, 5,
            'G', false, 80);
    MONSTER(MON_DEMON, "demon", MC_NECRO, 20, 4, MTFLAG_ATK_MELEE, 4, 2, 'd',
            false,
            80);
    MONSTER(MON_DEMON_LORD, "demon lord", MC_NECRO, 30, 2, MTFLAG_ATK_MELEE, 7,
            5, 'D',
            false, 80);
    MONSTER(MON_DEMON_KING, "demon king", MC_NECRO, 50, 0,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_SPELL, 9, 11, 'D', true,
            80);
    MONSTER(MON_ANGEL, "angel", MC_PALLADIN, 30, 0,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_TP, 6, 7, 'A',
            false, 20);
    MONSTER(MON_ARCHANGEL, "archangel", MC_PALLADIN, 35, -1,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_TP | MTFLAG_ATK_TIMEOUT, 10, 13, 'A',
            true, 20);
    MONSTER(MON_LESSER_MINION, "lesser minion", MC_NECRO, 20, 4,
            MTFLAG_ATK_MELEE, 4,
            2, 'm', false, 80);
    MONSTER(MON_MINION, "minion", MC_NECRO, 30, 2, MTFLAG_ATK_MELEE, 7, 5, 'M',
            false,
            80);
    MONSTER(MON_GREATER_MINION, "greater minion", MC_NECRO, 50, 0,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_SPELL, 9, 11, 'M', true,
            80);
    MONSTER(MON_TT, "tt", MC_NECRO, 75, -6,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_SPELL |
            MTFLAG_ATK_EXPDRAIN, 15, 25, '@',
            true, 0);
    MONSTER(MON_ESTEBAN, "esteban", MC_PALLADIN, 75, -6,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_TP | MTFLAG_INVIS,
            15, 25, '@', true, 0);
    MONSTER(MON_CHARLES, "charles", MC_EVOK, 75, -6,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_LEVITATE |
            MTFLAG_ATK_CONJURE, 15, 25, '@', true,
            0);
    MONSTER(MON_ZALE, "zale", MC_PALLADIN, 75, -6,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_DISENCHANT |
            MTFLAG_ATK_TIMEOUT, 15, 25, '@',
            true, 0);
    MONSTER(MON_ODYSSEUS, "odysseus", MC_NECRO, 75, -6,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_DESTROY, 15, 25, '@',
            true,
            0);
    MONSTER(MON_GREEN_DRAGON, "green dragon", MC_EVOK, 50, -2,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_CONJURE, 15, 13, 'D',
            true, 0);
    MONSTER(MON_CYAN_DRAGON, "cyan dragon", MC_NECRO, 50, -2,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_EXPDRAIN, 15, 13,
            'D', true, 0);
    MONSTER(MON_RED_DRAGON, "red dragon", MC_PALLADIN, 50, -2,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY, 15, 25, 'D', true,
            0);
    MONSTER(MON_YELLOW_DRAGON, "yellow dragon", MC_NECRO, 50, -2,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_INVIS, 15, 25, 'D',
            true, 0);
    MONSTER(MON_BLUE_DRAGON, "blue dragon", MC_EVOK, 50, -2,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_TIMEOUT, 15, 25, 'D',
            true,
            0);
    MONSTER(MON_WIZARD_OF_YENDOR, "Wizard of Yendor", MC_PALLADIN, 100, -5,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_TP | MTFLAG_BLOWBACK | MTFLAG_ATK_RAY,
            15, 21, '@', false, 0);
    MONSTER(MON_SPIRIT, "spirit", MC_NECRO, 20, 4,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY, 4,
            2, 'm', false, 80);
    MONSTER(MON_HIGHER_SPIRIT, "higher spirit", MC_NECRO, 30, 2,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY, 7, 5, 'S', false,
            80);
    MONSTER(MON_GHOST, "ghost", MC_NECRO, 50, 0,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_SPELL | MTFLAG_ATK_TIMEOUT,
            9, 11, 'S', true, 80);
    MONSTER(MON_ELF, "elf", MC_PALLADIN, 20, 4, MTFLAG_ATK_MELEE, 4,
            2, 'e', false, 80);
    MONSTER(MON_ELF_LORD, "elf lord", MC_PALLADIN, 30, 2, MTFLAG_ATK_MELEE, 7,
            5, 'E',
            false,
            80);
    MONSTER(MON_ELF_KING, "elf king", MC_PALLADIN, 50, 0,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_TP, 9, 11, 'E', true, 80);
    MONSTER(MON_DWARF, "dwarf", MC_EVOK, 20, 4, MTFLAG_ATK_MELEE, 4,
            2, 'd', false, 80);
    MONSTER(MON_DWARF_LORD, "dwarf lord", MC_EVOK, 30, 2, MTFLAG_ATK_MELEE, 7,
            5, 'D',
            false,
            80);
    MONSTER(MON_DWARF_KING, "dwarf king", MC_EVOK, 50, 0,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_TP, 9, 11, 'D', true, 80);
    MONSTER(MON_JUIBLEX, "Juiblex", MC_EVOK, 60, -4,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_RAY | MTFLAG_ATK_FIRE, 10, 17, '&',
            true, 20);
    MONSTER(MON_DEMOGORGON, "Demogorgon", MC_EVOK, 60, -4,
            MTFLAG_ATK_MELEE | MTFLAG_LEVITATE, 10, 17, '&', true, 20);
    MONSTER(MON_ANT, "ant", MC_NEUTRAL, 13, 4, MTFLAG_ATK_MELEE, 3, 2, 'a',
            false, 80);
    MONSTER(MON_FIRE_ANT, "fire ant", MC_NEUTRAL, 13, 4,
            MTFLAG_ATK_MELEE | MTFLAG_ATK_FIRE,
            3, 3, 'a', true, 20);


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

    return i;
}

/**
 * Adds the specified number (or 2d6 by default) of monsters to a level
 * @param dlvl Level at which to add the monsters
 * @param nb Number of monsters to add. If non-positive, will default to 2d6.
 */
void make_monsters(int dlvl, int nb)
{
    if (nb <= 0)
        nb = ndn(2, 6);

    for (int i = 0; i < nb; i++) {
        Monster *mon = malloc(sizeof(Monster));
        mon->type = &monster_types[choose_montype(dlvl)];
        mon->flags = 0;
        mon->hp = mon->type->max_hp;
        mon->freeze_timeout = 0;
        mon->spell_timeout = 0;
        mon->remembered = false;
        mon->remembered_pos = (Coord) {0, 0};

        mon->dlvl = dlvl;
        if (!find_tile(&mon->pos, dlvl, false, -1)) {
            print_to_log("Level %d full, cannot add monster", dlvl);
            free(mon);
            return;
        }

        add_to_linked_list(m_list, mon);
    }
}

/**
 * Returns the monster at the specified coordinates, or NULL if there is none
 * @param dlvl Level on which to search
 * @param pos Coordinates to search
 * @return The monster or NULL.
 */
Monster *find_mon_at(int dlvl, Coord pos)
{
    LinkedListNode *mon_node = m_list->head;

    if (!mon_node)
        return NULL; /* Be careful with that one ! */

    while (mon_node != NULL) {
        Monster *mon = (Monster *) mon_node->element;
        if (mon->pos.x == pos.x && mon->pos.y == pos.y && mon->dlvl == dlvl)
            return mon;

        mon_node = mon_node->next;
    }

    return NULL;
}

/*
 * check_dead: Check if a monster is dead and take appropriate action. Returns
 * whether the monster is dead.
 */
bool check_dead(Monster *target, bool rodney_killed)
{
    if (target->hp > 0)
        return false;

    if (rodney_killed) {
        rodney.magic_class_exp[target->type->magic_class] +=
                10 * target->type->difficulty;
        pline("You kill the %s.", target->type->name);
    } else {
        pline("The %s is killed.", target->type->name);
    }
    delete_from_linked_list(m_list, target);
    free(target);

    return true;
}

/**
 * Have a monster hit the player in melee. Assumes the player can move and
 * it placed right to hit.
 */
void mon_melee_attack(Monster *mon)
{
    if (ndn(mon->type->difficulty, 3) < 10 - rodney.ac) {
        pline("The %s hits!", mon->type->name);
        take_damage(ndn(3, mon->type->power / 3) - 2);
    } else {
        pline("The %s misses!", mon->type->name);
    }

    if (mon->type->flags & MTFLAG_ATK_FREEZE) {
        if (rand_int(1, 10) == 1) {
            pline("The %s freezes you!", mon->type->name);
            rodney.freeze_timeout += ndn(10, 10);
        }
    }
}


/**
 * Have a monster attack the player in a ranged attack. Assumes the monster
 * has a line of sight to the player, and that it is currently able to hit.
 */
void mon_ranged_attack(Monster *mon)
{
    if (ndn(mon->type->difficulty, 3) < 10 - rodney.ac) {
        pline("The %s casts a spell. The bolt hits you!", mon->type->name);
        take_damage(ndn(3, mon->type->power / 3) - 2);
    } else {
        pline("The %s casts a spell. The bolt whizzes by you.", mon->type->name);
    }
}


/**
 * Tries to make a monster attack. Returns true iff it was possible and the
 * monster attacked.
 */
bool monster_attack(Monster *mon)
{
    // Try to attack in melee
    for (int i_direction = 0; i_direction < 8; i_direction++) {
        Coord neighbor = get_neighbor(mon->pos, i_direction);
        if (!valid_coordinates(neighbor))
            continue;
        if (neighbor.x == rodney.pos.x && neighbor.y == rodney.pos.y) {
            mon_melee_attack(mon);
            return true;
        }
    }

    // Try to do a ranged attack
    if (mon->type->flags & MTFLAG_ATK_SPELL && mon->spell_timeout < 0) {
        if (is_visible(rodney.dlvl, mon->pos, rodney.pos, NULL, false)) {
            mon_ranged_attack(mon);
        }
    }

    return false;
}

/**
 * Make a monster act for the turn: give it a chance to attack, or move it
 * around the level. Monsters who see rodney or remember his position will move
 * towards him, others will simply wander around.
 * This function assumes the monster is able to move.
 */
void monster_move(Monster *mon)
{
    Coord target;
    Coord new;

    // Update remembered position
    if (!is_visible(mon->dlvl, mon->pos, rodney.pos, NULL, false)) {
        if (mon->remembered &&
            is_visible(mon->dlvl, mon->pos, mon->remembered_pos, NULL,
                       false) &&
            mon->remembered_pos.x != rodney.pos.x &&
            mon->remembered_pos.y != rodney.pos.y) {
            mon->remembered = false;
        }
    } else {
        mon->remembered = true;
        mon->remembered_pos = rodney.pos;
    }

    if (!mon->remembered) {
        int direction = rand_int(0, 7);
        Coord tentative = get_neighbor(mon->pos, direction);
        if (valid_coordinates(tentative) &&
            IS_WALKABLE(maps[mon->dlvl][tentative.x][tentative.y]) &&
            !find_mon_at(mon->dlvl, tentative)) {
            mon->pos = tentative;
        }

        return;
    } else {
        target = mon->remembered_pos;
    }

    /* First try finding a path clear of monsters; if it fails, try to
     * find a path with monsters */
    if (dijkstra(rodney.dlvl, mon->pos, target, &new, false) ||
        dijkstra(rodney.dlvl, mon->pos, target, &new, true)) {
        if (!find_mon_at(rodney.dlvl, new)) {
            mon->pos = new;
        }
    }
}


/**
 * Move each monster on the level and get them a chance to attack. Only
 * monsters that are on the current level, that can move, and that are not
 * frozen/etc. will do something. Cooldowns will be decreased.
 */
void tick_monsters()
{
    LinkedListNode *cur_node;
    Monster *mon;

    for (cur_node = m_list->head; cur_node != NULL; cur_node = cur_node->next) {
        mon = cur_node->element;
        if (mon->dlvl != rodney.dlvl || mon->type->flags & MTFLAG_IMMOBILE)
            continue;

        if (mon->freeze_timeout > 0) {
            mon->freeze_timeout--;
        } else {
            mon->flags &= ~MF_FROZEN;
        }

        if (mon->spell_timeout > 0)
            mon->spell_timeout--;

        if (!monster_attack(mon))
            monster_move(mon);
    }
}