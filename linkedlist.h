//
// Created by Matthieu Felix on 17/09/2016.
//

#ifndef OPMORL_2_0_LINKEDLIST_H
#define OPMORL_2_0_LINKEDLIST_H


typedef struct s_linked_list_node {
    void *element;
    struct s_linked_list_node *next;
} LinkedListNode;


/*
 * Linked list structure to simplify adding/removing elements from an empty list.
 */
typedef struct {
    LinkedListNode *head;
    LinkedListNode *cur_pos;
    LinkedListNode *prev_pos;
} LinkedList;


LinkedList *new_linked_list();
void delete_linked_list(LinkedList *linked_list);

int is_empty(LinkedList *linked_list);
void append(LinkedList *linked_list, void *item);
int delete(LinkedList *linked_list, void *item);


#endif //OPMORL_2_0_LINKEDLIST_H
