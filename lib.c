/*
 *  lib.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

/* This includes chained list stuff */

#include "opmorl.h"

/*
 * Returns a random integer in [min,max]
 */
int rand_int(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void add_object(Object *obj)
{
    append(o_list, obj);
}

void add_monster(Monster *mon)
{
    append(m_list, mon);
}

Monster *find_mon_at(int x, int y, int level)
{
    LinkedListNode *mon_node = m_list->head;

    if (!mon_node)
        return NULL; /* Be careful with that one ! */

    while(mon_node != NULL) {
        Monster *mon = (Monster *) mon_node->element;
        if(mon->posx == x && mon->posy == y && mon->level == level)
            return mon;

        mon_node = mon_node->next;
    }

    return NULL;
}

Object *find_obj_at(int x, int y, int level)
{
    LinkedListNode *obj_node = m_list->head;

    if (!obj_node)
        return NULL; /* Be careful with that one ! */

    while(obj_node != NULL) {
        Object *obj = (Object *) obj_node->element;
        if(obj->posx == x && obj->posy == y && obj->level == level)
            return obj;

        obj_node = obj_node->next;
    }

    return NULL;
}

void rm_mon_at(int x, int y, int level)
{
    Monster *mon;

    if((mon = find_mon_at(x, y, level)) == NULL)
        return;

    delete(m_list, mon);
}

void rm_obj_at(int x, int y, int level)
{
    Object *obj;

    if((obj = find_obj_at(x, y, level)) == NULL)
        return;

    delete(o_list, obj);
}

