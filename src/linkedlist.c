/*
 *  linkedlist.c
 *  OPMORL 2
 *
 *  Created by Matthieu Felix on 17/09/2016.
 *  Copyright 2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include <stdlib.h>
#include "linkedlist.h"

/*
 * Returns an empty allocated linked list
 */
LinkedList *new_linked_list()
{
    LinkedList *ret = malloc(sizeof(LinkedList));
    ret->head = NULL;
    ret->length = 0;

    return ret;
}

/**
 * De-allocates a linked list structure
 */
void delete_linked_list(LinkedList *linked_list)
{
    LinkedListNode *cur = linked_list->head;
    LinkedListNode *to_delete;

    while(cur != NULL) {
        to_delete = cur;
        cur = cur->next;
        free(to_delete);
    }

    free(linked_list);
}

/**
 * Checks if a linked list is empty
 */
int is_empty(LinkedList *linked_list)
{
    return linked_list->length == 0;
}


/**
 * Reverses a linked list
 */
void reverse_linked_list(LinkedList *linked_list)
{
    if (linked_list->length < 2)
        return;

    LinkedListNode *next = NULL;
    LinkedListNode *cur = linked_list->head->next;
    LinkedListNode *prev = linked_list->head;

    linked_list->head->next = NULL;

    while (cur != NULL) {
        next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }

    linked_list->head = prev;
}


/**
 * Adds an element at the beginning of a linked list.
 */
void add_to_linked_list(LinkedList *linked_list, void *item)
{
    LinkedListNode *new = malloc(sizeof(LinkedListNode));
    new->element = item;
    new->next = linked_list->head;
    linked_list->head = new;
    linked_list->length++;
}

/**
 * Deletes the first instance of item from the list. Returns 1 if successful,
 * 0 if the element was not found.
 */
int delete_from_linked_list(LinkedList *linked_list, void *item)
{
    LinkedListNode *cur = linked_list->head;
    LinkedListNode *prev = linked_list->head;

    if (is_empty(linked_list))
        /* Tried to delete an element from an empty list */
        return 0;

    if(cur->element == item) {
        linked_list->head = linked_list->head->next;
        free(cur);
        linked_list->length--;
        return 1;
    }

    while(cur->element != item) {
        prev = cur;
        cur = cur->next;

        if(cur == NULL)
            return 0;
    }

    prev->next = cur->next;
    free(cur);
    linked_list->length--;

    return 1;
}


/**
 * Generates a linked list from an array.
 * @param compress If false, NULL elements will be inserted for empty array
 * elements.
 */
LinkedList *array_to_linked_list(void **array, int size, bool compress)
{
    LinkedList *ret = new_linked_list();

    for (int i = 0; i < size; i++) {
        if (!compress || array[i] != NULL)
            add_to_linked_list(ret, array[i]);
    }

    reverse_linked_list(ret);
    return ret;
}


/**
 * Checks whether an element is in a linked list.
 */
bool is_in_linked_list(LinkedList *list, void *element)
{
    LinkedListNode *cur = list->head;

    while (cur != NULL) {
        if (cur->element == element)
            return true;
        cur = cur->next;
    }

    return false;
}
