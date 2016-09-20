//
// Created by Matthieu Felix on 18/09/2016.
//

#include "opmorl.h"

/*
 * Converts a char into an inventory letter. Returns -1 if unknown.
 */
int letter_to_slot(char c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a';

    if (c >= 'A' && c <= 'Z')
        return 26 + c - 'A';

    else
        return -1;
}

char slot_to_letter(int i)
{
    if (i < 0 || i > 51)
        return '?';
    else if (i < 26)
        return (char) ('a' + i);
    else
        return (char) ('A' + i);
}

/*
 * Adds an object to inventory. Returns -1 if the inventory is full, or the allocated slot otherwise.
 */
int add_to_inventory(Object *obj)
{
    int slot;
    for (slot = 0; slot < INVENTORY_SIZE && rodney.inventory[slot] != NULL; slot++);

    if (slot >= INVENTORY_SIZE) // No free slot
        return -1;

    rodney.inventory[slot] = obj;
    return slot;
}


void delete_from_inventory(Object *obj)
{
    int slot;
    for (slot = 0; slot < INVENTORY_SIZE; slot++) {
        if (rodney.inventory[slot] == obj) {
            rodney.inventory[slot] = NULL;
            break;
        }
    }
}


int pickup()
{
    LinkedList *cur_objects;
    Object *ret;
    int slot;
    int elapsed = 0;

    cur_objects = find_objs_at(rodney.posx, rodney.posy, rodney.dlvl);
    if (cur_objects->length == 0) {
        pline("There is nothing here");
        return 0;
    } else if (cur_objects->length == 1)
        ret = cur_objects->head->element;
    else
        ret = select_object(cur_objects);

    delete_linked_list(cur_objects);

    if (ret != NULL) {
        if (ret->type->class->o_class_flag == OT_MONEY) {
            rodney.gold += ret->amount;
            delete_from_linked_list(o_list, ret);
            free(ret);
        } else if ((slot = add_to_inventory(ret)) != -1) {
            pline("%c - %s", slot_to_letter(slot), ret->type->name);
            delete_from_linked_list(o_list, ret);
            elapsed = 1;
        } else
            pline("Your pack is full.");
    } else
        pline("Never mind.");

    return elapsed;
}

int drop()
{
    LinkedList *inventory_list = array_to_linked_list((void **) rodney.inventory, INVENTORY_SIZE, false);
    Object *to_drop = select_object(inventory_list);

    if (to_drop == NULL) {
        pline("Never mind.");
        delete_linked_list(inventory_list);
        return 0;
    }

    to_drop->posx = rodney.posx;
    to_drop->posy = rodney.posy;
    to_drop->level = rodney.dlvl;

    add_to_linked_list(o_list, to_drop);
    delete_from_inventory(to_drop);

    delete_linked_list(inventory_list);

    return 1;
}

/*
 * Temporary function before inventory display is completed
 */
int dump_inventory()
{
    print_to_log("Inventory dump\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (rodney.inventory[i] != NULL)
            print_to_log("    %c - %s\n", slot_to_letter(i), rodney.inventory[i]->type->name);
    }
    pline("Inventory dumped");

    return 0;
}
