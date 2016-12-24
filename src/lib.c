/*
 *  lib.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

/* This includes chained list stuff */

#include "opmorl.h"
#include "lib.h"

/**
 * Returns a random integer in the specified range
 * @param min, max The inclusive bounds of the range
 * @return A random integer in that range
 */
int rand_int(int min, int max)
{
    if (max < min)
        return min;

    return rand() % (max - min + 1) + min;
}

/**
 * Minimum
 * @param a, b Two ints
 * @return The minimum of the two ints
 */
int min(int a, int b)
{
    return a > b ? b : a;
}

/**
 * Maximum
 * @param a, b Two ints
 * @return The maximum of the two ints
 */
int max(int a, int b)
{
    return a > b ? a : b;
}

/**
 * Absolute value (integer)
 * @param a An int
 * @return The absolute value of the int
 */
int abs(int a)
{
    return a < 0 ? -a : a;
}

/**
 * Absolute value (float)
 * @param a A double
 * @return The absolute value of the double
 */
double abs_d(double a)
{
    return a < 0 ? -a : a;
}

/**
 * Sign
 * @param a An integer
 * @return The sign of the integer
 */
int sign(int a)
{
    if (a >= 0)
        return 1;
    else
        return -1;
}

/**
 * Copy a string, ensuring the resulting string is null-terminated.
 * @param dest The destination array
 * @param src The source string
 * @param n The maximum usable size of the destionation array
 */
void strncpy_pad(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\0';
}

/**
 * Returns a random number following a dice distribution
 * @param num_dice The number of dice (>= 1)
 * @param faces The number of faces per dice (>= 1)
 * @return A (num_dice)d(faces) number.
 */
int ndn(int num_dice, int faces)
{
    int acc = 0;
    for (int i = 0; i < num_dice; i++)
        acc += rand_int(1, faces);

    return acc;
}

/**
 * Returns the direction represented by a given letter, with the following map:
 * y  k  u
 *  \ | /
 * h- . -l
 *  / | \
 * b  j  n
 * @param letter The letter to convert
 * @return A Coord struct representing the direction
 */
Coord letter_to_direction(char letter)
{
    Coord res = {0, 0};

    if (letter == 'y' || letter == 'h' || letter == 'b')
        res.y--;
    if (letter == 'u' || letter == 'l' || letter == 'n')
        res.y++;
    if (letter == 'y' || letter == 'k' || letter == 'u')
        res.x--;
    if (letter == 'b' || letter == 'j' || letter == 'n')
        res.x++;

    return res;
}

/**
 * Adds a coordinate and a direction. Checks that the resulting coordinate is
 * valid.
 * @param a Coordinate
 * @param b Direction
 * @return The sum
 */
Coord coord_add(Coord a, Coord b)
{
    Coord res = (Coord) {a.x + b.x, a.y + b.y};

    if (res.x < 0)
        res.x = 0;
    else if (res.x >= LEVEL_HEIGHT)
        res.x = LEVEL_HEIGHT - 1;

    if (res.y < 0)
        res.y = 0;
    else if (res.y >= LEVEL_WIDTH)
        res.y = LEVEL_WIDTH - 1;

    return res;
}

/**
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

    while (cur != NULL) {
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

    if (cur->element == item) {
        linked_list->head = linked_list->head->next;
        free(cur);
        linked_list->length--;
        return 1;
    }

    while (cur->element != item) {
        prev = cur;
        cur = cur->next;

        if (cur == NULL)
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
