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

int min(int a, int b)
{
    return a > b ? b : a;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int abs(int a)
{
    return a < 0 ? -a : a;
}

double abs_d(double a)
{
    return a < 0 ? -a : a;
}

int sign(int a)
{
    if (a >= 0)
        return 1;
    else
        return -1;
}

void strncpy_pad(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\0';
}

void add_object(Object *obj)
{
    add_to_linked_list(o_list, obj);
}

void add_monster(Monster *mon)
{
    add_to_linked_list(m_list, mon);
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

LinkedList *find_objs_at(int x, int y, int level)
{
    LinkedList *ret = new_linked_list();
    LinkedListNode *obj_node = o_list->head;

    if (!obj_node)
        return NULL; /* Be careful with that one ! */

    while (obj_node != NULL) {
        Object *obj = (Object *) obj_node->element;
        if (obj->posx == x && obj->posy == y && obj->level == level)
            add_to_linked_list(ret, obj);

        obj_node = obj_node->next;
    }

    return ret;
}

void rm_mon_at(int x, int y, int level)
{
    Monster *mon;

    if ((mon = find_mon_at(x, y, level)) == NULL)
        return;

    delete_from_linked_list(m_list, mon);
}
