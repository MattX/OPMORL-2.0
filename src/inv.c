//
// Created by Matthieu Felix on 18/09/2016.
//

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