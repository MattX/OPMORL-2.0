/*
 *  linkedlist.h
 *  OPMORL 2
 *
 *  Created by Matthieu Felix on 17/09/2016.
 *  Copyright 2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#ifndef OPMORL_2_0_LINKEDLIST_H
#define OPMORL_2_0_LINKEDLIST_H

#include "stdbool.h"

typedef struct s_linked_list_node {
    void *element;
    struct s_linked_list_node *next;
} LinkedListNode;


/**
 * Linked list structure to simplify adding/removing elements from an empty
 * list (since head can easily be changed).
 */
typedef struct {
    LinkedListNode *head;
    int length;
} LinkedList;


LinkedList *new_linked_list();
void delete_linked_list(LinkedList *linked_list);

int is_empty(LinkedList *linked_list);

void reverse_linked_list(LinkedList *linked_list);

void add_to_linked_list(LinkedList *linked_list, void *item);

int delete_from_linked_list(LinkedList *linked_list, void *item);

LinkedList *array_to_linked_list(void **array, int size, bool compress);

bool is_in_linked_list(LinkedList *list, void *element);

#endif //OPMORL_2_0_LINKEDLIST_H
