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
 * u  k  i
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

    if (letter == 'u' || letter == 'h' || letter == 'b')
        res.y--;
    if (letter == 'i' || letter == 'l' || letter == 'n')
        res.y++;
    if (letter == 'u' || letter == 'k' || letter == 'i')
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