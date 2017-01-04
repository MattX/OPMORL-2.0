/*
 *  inv.c
 *  OPMORL 2
 *
 *  Created by Matthieu Felix on 18/09/2016.
 *  Copyright 2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"

/*
 * letter_to_slot: Converts a char into an inventory letter. Returns -1 if
 * the letter is not a valid slot (ie not in a-zA-Z).
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

/*
 * slot_to_letter: Converts a slot number to a letter for display or selection
 * purposes. The slot number has to be between 0 and 51.
 */
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
 * add_to_inventory: Adds an object to inventory. Returns -1 if the inventory
 * is full, or the allocated slot number otherwise.
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

/*
 * delete_from_inventory: Remove the given object from the inventory of the
 * player, if it is present.
 */
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

/*
 * pickup: Ask the player which of the items on the ground to pick up. If a
 * valid selection is made, move that item from the ground to the inventory.
 * If the object is gold, it is destroyed and its amount is added to the
 * gold field in the player struct.
 */
int pickup()
{
    LinkedList *cur_objects;
    Object *ret;
    int slot;
    int elapsed = 0;

    cur_objects = find_objs_at(rodney.dlvl, rodney.pos);
    if (cur_objects->length == 0) {
        pline("There is nothing here.");
        return 0;
    } else if (cur_objects->length == 1)
        ret = cur_objects->head->element;
    else
        ret = select_object(cur_objects);

    delete_linked_list(cur_objects);

    if (ret != NULL) {
        if (ret->type->class->o_class_flag == OT_MONEY) {
            rodney.gold += ret->enchant;
            pline("%d gold pieces.", ret->enchant);
            delete_from_linked_list(o_list, ret);
            free(ret);
        } else if ((slot = add_to_inventory(ret)) != -1) {
            pline("%c - %s %s.", slot_to_letter(slot),
                  indefinite_article(object_name(ret)), object_name(ret));
            delete_from_linked_list(o_list, ret);
            elapsed = 1;
        } else
            pline("Your pack is full.");
    } else
        pline("Never mind.");

    return elapsed;
}

/**
 * Ask the player for an item to drop, and drop it. Will return 1 if an item
 * was dropped or 0 if the player cancelled the action.
 */
int drop()
{
    LinkedList *inventory_list = array_to_linked_list(
            (void **) rodney.inventory,
            INVENTORY_SIZE, false);
    Object *to_drop = select_object(inventory_list);
    int elapsed = 0;

    if (to_drop == NULL) {
        pline("Never mind.");
    } else if (to_drop == rodney.wielded) {
        pline("You can't drop something you're wielding!");
    } else if (to_drop == rodney.helm || to_drop == rodney.body_armor) {
        pline("You can't drop something you're wearing!");
    } else {
        to_drop->pos = rodney.pos;
        to_drop->dlvl = rodney.dlvl;

        add_to_linked_list(o_list, to_drop);
        delete_from_inventory(to_drop);

        elapsed = 1;
    }

    delete_linked_list(inventory_list);
    return elapsed;
}

/**
 * Display the player's inventory.
 */
void inventory()
{
    LinkedList *inv = array_to_linked_list((void **) rodney.inventory,
                                           INVENTORY_SIZE, false);
    select_object(inv);
    delete_linked_list(inv);
}

/**
 * Asks the player for an item to wield and weilds it if a proper choice is
 * made. Fails if something is already being weilded.
 */
int wield()
{
    if (rodney.wielded != NULL) {
        pline("You are already wielding a %s. Unwield it with X.",
              object_name(rodney.wielded));
        return 0;
    }

    pline("What do you want to wield?");
    LinkedList *inv = array_to_linked_list((void **) rodney.inventory,
                                           INVENTORY_SIZE, false);
    Object *selected = select_object(inv);
    delete_linked_list(inv);

    if (selected != NULL) {
        rodney.wielded = selected;
        pline("You are now wielding a %s.", object_name(rodney.wielded));
        return 1;
    } else {
        pline("Never mind.");
        return 0;
    }
}

/**
 * Un-wields the currently wielded item, if there is one.
 */
int unwield()
{
    if (rodney.wielded == NULL) {
        pline("You are not wielding anything.");
        return 0;
    }

    rodney.wielded = NULL;
    pline("You are now empty-handed.");
    return 1;
}


/**
 * Returns the change in player AC when the object is put on/removed. The value
 * returned will be positive (ie it is the AC increase when the object is
 * removed).
 */
int ac_change(Object *obj)
{
    if (!(obj->type->class->o_class_flag & (OT_BODY_ARMOR | OT_HELM)))
        return 0;

    else
        return 2 + obj->type->power / 3 + obj->enchant;
}


/**
 * Asks for an item to wear, wears it if the selection is valid.
 */
int wear()
{
    pline("What do you want to wear?");

    LinkedList *inv = array_to_linked_list((void **) rodney.inventory,
                                           INVENTORY_SIZE, false);
    Object *selected = select_object(inv);
    delete_linked_list(inv);

    if (selected == NULL) {
        pline("Never mind.");
        return 0;
    }

    switch (selected->type->class->o_class_flag) {
    case OT_BODY_ARMOR:
        if (rodney.body_armor != NULL) {
            pline("You are already wearing body armor. Take it off with T.");
            return 0;
        } else {
            rodney.body_armor = selected;
            rodney.ac -= ac_change(rodney.body_armor);
            pline("You are now wearing a %s.", object_name(selected));
            return 1;
        }

    case OT_HELM:
        if (rodney.helm != NULL) {
            pline("You are already wearing a helm. Take it off with T.");
            return 0;
        } else {
            rodney.helm = selected;
            rodney.ac -= ac_change(rodney.helm);
            pline("You are now wearing a %s.", object_name(selected));
            return 1;
        }

    default:
        pline("You can't wear that.");
        return 0;
    }

    return 0; // Should not reach;
}

/**
 * Asks confirmation and removes armor.
 */
int take_off_armor()
{
    int confirm;

    if (rodney.helm == NULL && rodney.body_armor == NULL) {
        pline("You are not wearing anything. Fortunately, there are no kids around.");
        return 0;
    }
    if (rodney.helm != NULL) {
        confirm = yes_no("Do you want to take off your %s?",
                         object_name(rodney.helm));
        if (confirm) {
            pline("You were wearing a %s.", object_name(rodney.helm));
            rodney.ac += ac_change(rodney.helm);
            rodney.helm = NULL;
            return 1;
        }
    }
    if (rodney.body_armor != NULL) {
        confirm = yes_no("Do you want to take off your %s?",
                         object_name(rodney.body_armor));
        if (confirm) {
            pline("You were wearing a %s.", object_name(rodney.body_armor));
            rodney.ac += ac_change(rodney.body_armor);
            rodney.body_armor = NULL;
            return 1;
        }
    }

    return 0; // Should not reach here
}

/**
 * Returns whether an inventory object has the required effect.
 */
bool has_inventory_effect(MixinType effect)
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (rodney.inventory[i] == NULL)
            continue;
        if (has_mixin(rodney.inventory[i]->type, effect))
            return true;
    }

    return false;
}