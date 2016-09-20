/*
 *  pline.c
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010 OPMORL 2 dev team. All rights reserved.
 *
 */

#include "opmorl.h"

// Maximum height of full screen windows like inventory selection
#define WINDOW_HEIGHT 18

char get_input()
{
    return (char) getch();
}

void display_everything()
{
    display_stats(); /* The stuff like health points, level et alii */
    show_env_messages();
    if (!line_displayed)
        clear_msg_line();
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
            if (visibility_map[rodney.dlvl][i][j] == TS_UNDISCOVERED) {
                mvaddch(i + 1, j, ' ');
                continue;
            }

            if (visibility_map[rodney.dlvl][i][j] == TS_SEEN)
                attron(A_BOLD);

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

            if (visibility_map[rodney.dlvl][i][j] == TS_SEEN)
                attroff(A_BOLD);
        }
    }

    attroff(COLOR_PAIR(DEFAULT));
    /* Objects */
    // TODO: add memory for object positions
    if (obj_node)
        do {
            Object *obj = (Object *) obj_node->element;
            if (obj->level != rodney.dlvl || visibility_map[rodney.dlvl][obj->posx][obj->posy] == TS_UNDISCOVERED)
                continue;

            attron(COLOR_PAIR(obj->type->color));
            mvaddch(obj->posx + 1, obj->posy, obj->type->class->symbol);
            attroff(COLOR_PAIR(obj->type->color));
        } while ((obj_node = obj_node->next));

    /* Monsters */
    if (mon_node)
        do {
            Monster *mon = (Monster *) mon_node->element;
            if (mon->level != rodney.dlvl || visibility_map[rodney.dlvl][mon->posx][mon->posy] != TS_SEEN)
                continue;

            attron(COLOR_PAIR(magic_class_colors[mon->type->magic_class]));
            mvaddch(mon->posx + 1, mon->posy, mon->type->symbol);
            attroff(COLOR_PAIR(magic_class_colors[mon->type->magic_class]));
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
    mvprintw(getmaxy(stdscr) - 1, 0, "Dlvl:%d\t$:%d\tHP:%d(%d)\tT:%d", rodney.dlvl + 1, rodney.gold, rodney.hp,
             rodney.max_hp, turn);
}

void clear_msg_line()
{
    move(0, 0);
    clrtoeol();
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

// TODO: object selection window for several objects

/*
 * Selects the next node with non-null element. If skipped is not none, if will be incremented by the number
 * of skipped elements
 */
LinkedListNode *find_not_null(LinkedListNode *node, int *skipped)
{
    while (node != NULL && node->element == NULL) {
        node = node->next;
        if (skipped != NULL)
            *skipped += 1;
    }

    return node;
}

/*
 * Displays an object selection window for one object. There may be NULL elements in the linked list, corresponding
 * to empty slots from the inventory / a container.
 */
Object *select_object(LinkedList *objects)
{
    LinkedListNode *top = NULL; // Which object is the first on the current page
    LinkedListNode *next_top = objects->head; // Which object is the first on the next page
    LinkedListNode *cur = NULL;
    Object *selected = NULL;
    int slot = -1;
    int non_null_i = 0;

    while (selected == NULL) {
        top = next_top;
        cur = find_not_null(top, &non_null_i);

        for (int i = 0; i < WINDOW_HEIGHT - 1; i++) {
            if (cur == NULL)
                break;

            mvprintw(i + 1, 0, "%c - %s", slot_to_letter(non_null_i), ((Object *) cur->element)->type->name);
            clrtoeol();

            cur = cur->next;
            non_null_i++;
            cur = find_not_null(cur, &non_null_i);
        }
        next_top = cur;

        if (next_top != NULL)
            mvprintw(WINDOW_HEIGHT, 0, "<space> for next page, - to cancel");
        else
            mvprintw(WINDOW_HEIGHT, 0, "- to cancel");
        clrtoeol();

        while (1) {
            int selection = getch();
            if (selection == '-')
                return NULL;
            else if (selection == ' ' && next_top != NULL)
                break;
            else if ((slot = letter_to_slot((char) selection)) != -1) {
                cur = top;
                for (int i = 0; i < slot && cur != NULL; i++)
                    cur = cur->next;

                if (cur != NULL) {
                    selected = cur->element;
                    break;
                } else
                    pline("You don't have item %c", selection);
            } else
                pline("Invalid command");
        }
    }

    return selected;
}

void print_to_log(char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
}