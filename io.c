/*
 *  pline.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"


char get_input()
{
    return (char) getch();
}

void display_everything()
{
    display_stats(); /* The stuff like health points, level et alii */
    display_map();
}

void display_map()
{
    int i, j;
    LinkedListNode *obj_node = o_list->head;
    LinkedListNode *mon_node = m_list->head;

    attron(COLOR_PAIR(DEFAULT));
    for (i = 0; i < LEVEL_HEIGHT; i++) {
        for (j = 0; j < LEVEL_WIDTH; j++) {
            switch (lvl_map[rodney.dlvl][i][j]) {
            case T_CLOSED_DOOR:
                mvaddch(i + 1, j, '+');
                break;
            case T_OPEN_DOOR:
                mvaddch(i + 1, j, '-');
                break;
            case T_CORRIDOR:
                //attron(COLOR_PAIR(CLR_YELLOW));
                mvaddch(i + 1, j, '.');
                //attroff(COLOR_PAIR(CLR_YELLOW));
                break;
            case T_WALL:
                mvaddch(i + 1, j, '#');
                break;
            case T_FLOOR:
                mvaddch(i + 1, j, '.');
                break;
            case T_STAIRS_UP:
                mvaddch(i + 1, j, '<');
                break;
            case T_STAIRS_DOWN:
                mvaddch(i + 1, j, '>');
                break;
            case T_GROUND:
                mvaddch(i + 1, j, ' ');
                break;
            }
        }
    }

    attroff(COLOR_PAIR(DEFAULT));
    /* Objects */
    if (obj_node)
        do {
            Object *obj = (Object *) obj_node->element;

            attron(COLOR_PAIR(obj->color));
            mvaddch(obj->posx + 1, obj->posy, obj->symbol);
            attroff(COLOR_PAIR(obj->color));
        } while ((obj_node = obj_node->next));

    /* Monsters */
    if (mon_node)
        do {
            Monster *mon = (Monster *) mon_node->element;

            attron(COLOR_PAIR(mon->color));
            mvaddch(mon->posx + 1, mon->posy, mon->symbol);
            attroff(COLOR_PAIR(mon->color));
        } while ((mon_node = mon_node->next));

    /* Rodney */
    attron(COLOR_PAIR(rodney.color));
    attron(A_BOLD);
    mvaddch(rodney.posx + 1, rodney.posy, '@');
    attroff(A_BOLD);
    attroff(COLOR_PAIR(rodney.color));

    move(rodney.posx + 1, rodney.posy);
}

void display_stats()
{
    mvprintw(getmaxy(stdscr) - 2, 0, "St:%d Dx:%d Co:%d In:%d Wi:%d Ch:%d", rodney.strength, rodney.dexterity,
             rodney.constitution, rodney.intelligence, rodney.wisdom, rodney.charisma);
    mvprintw(getmaxy(stdscr) - 1, 0, "Dlvl:%d\t$:%d\tHP:%d(%d)", rodney.dlvl + 1, rodney.gold, rodney.hp,
             rodney.max_hp);
}

void va_pline(char *format, va_list args)
{
    if (line_displayed) {
        mvprintw(0, last_col, " --more--");
        getch();
    }

    line_displayed = 1;

    move(0, 0);
    vwprintw(stdscr, format, args);
    last_col = getcurx(stdscr);

    clrtoeol();
}

void pline(char *format, ...)
{
    va_list args;
    va_start(args, format);
    va_pline(format, args);
    va_end(args);
}

int yes_no(char *format, ...)
{
    char append[] = " (y/n) ";
    char *new_format;
    int rep = 0;
    size_t new_format_len = strlen(format) + strlen(append) + 1;

    new_format = malloc(new_format_len * sizeof(char));
    snprintf(new_format, new_format_len, "%s%s", format, append);

    va_list args;
    va_start(args, format);
    va_pline(new_format, args);
    va_end(args);

    free(new_format);

    while (1) {
        rep = getch();
        if (rep == 'y' || rep == 'Y')
            return 1;
        if (rep == 'n' || rep == 'N')
            return 0;
    }
}

void print_to_log(char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
}