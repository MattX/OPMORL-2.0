//
// Created by Matthieu Felix on 17/09/2016.
//

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

/*
 * Deallocates a linked list structre
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

int is_empty(LinkedList *linked_list)
{
    return linked_list->length == 0;
}

/*
 * Adds an element to a linked list.
 */
void append(LinkedList *linked_list, void *item)
{
    LinkedListNode *cur = linked_list->head;
    LinkedListNode *new = malloc(sizeof(LinkedListNode));
    new->element = item;
    new->next = linked_list->head;
    linked_list->head = new;
    linked_list->length++;
}

/*
 * Deletes the first instance of item from the list. Returns 1 if successful, 0 if the element was not found.
 */
int delete(LinkedList *linked_list, void *item)
{
    LinkedListNode *cur = linked_list->head;
    LinkedListNode *prev = linked_list->head;

    if(linked_list->head == NULL)
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