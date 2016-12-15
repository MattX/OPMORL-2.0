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

/**
 * Returns a random integer in [min,max]
 */
int rand_int(int min, int max)
{
    if (max < min)
        return min;

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

int ndn(int num_dice, int faces)
{
    int acc = 0;
    for (int i = 0; i < num_dice; i++)
        acc += rand_int(1, faces);

    return acc;
}