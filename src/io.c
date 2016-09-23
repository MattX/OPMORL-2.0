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

/*
 * display_map: Display a map of the current level.
 */
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
            if (obj->level != rodney.dlvl ||
                visibility_map[rodney.dlvl][obj->posx][obj->posy] ==
                TS_UNDISCOVERED)
                continue;

            attron(COLOR_PAIR(obj->type->color));
            mvaddch(obj->posx + 1, obj->posy, obj->type->class->symbol);
            attroff(COLOR_PAIR(obj->type->color));
        } while ((obj_node = obj_node->next) != NULL);

    /* Monsters */
    if (mon_node)
        do {
            Monster *mon = (Monster *) mon_node->element;
            if (mon->level != rodney.dlvl ||
                visibility_map[rodney.dlvl][mon->posx][mon->posy] != TS_SEEN)
                continue;

            attron(COLOR_PAIR(magic_class_colors[mon->type->magic_class]));
            mvaddch(mon->posx + 1, mon->posy, mon->type->symbol);
            attroff(COLOR_PAIR(magic_class_colors[mon->type->magic_class]));
        } while ((mon_node = mon_node->next) != NULL);

    /* Rodney */
    attron(A_BOLD);
    mvaddch(rodney.posx + 1, rodney.posy, '@');
    attroff(A_BOLD);

    move(rodney.posx + 1, rodney.posy);
}

/*
 * display_stats: Display player stats at the bottom of the screen.
 */
void display_stats()
{
    int best_class = 0;
    for (int i = 0; i < NB_MAGIC_CLASSES; i++)
        if (rodney.magic_class_exp[i] > rodney.magic_class_exp[best_class])
            best_class = i;

    mvprintw(getmaxy(stdscr) - 2, 0, "Rodney the %s\t\tAC: %d",
             magic_class_names[best_class], rodney.ac);
    mvprintw(getmaxy(stdscr) - 1, 0, "Dlvl:%d\t$:%d\tHP:%d(%d)\tT:%d",
             rodney.dlvl + 1, rodney.gold, rodney.hp,
             rodney.max_hp, turn);
}

/*
 * clear_msg_line: Clear the message line on top of the screen.
 */
void clear_msg_line()
{
    move(0, 0);
    clrtoeol();
}

/*
 * va_pline: Print a line at the top of the screen. Supports printf variable
 * arguments, passed as a va_list. This is to allow calling from other
 * functions.
 */
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

/*
 * pline: Print a line at the top of the screen. Supports printf variable
 * arguments.
 */
void pline(char *format, ...)
{
    va_list args;
    va_start(args, format);
    va_pline(format, args);
    va_end(args);
}

/*
 * yes_no: Ask a yes-no question.
 */
bool yes_no(char *format, ...)
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
            return true;
        if (rep == 'n' || rep == 'N')
            return false;
    }

    line_displayed = 0; // To disable additional --more--
}

// TODO: object selection window for several objects

/*
 * Selects the next node with non-null element. If skipped is not none, if will
 * be incremented by the number of skipped elements
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
 * Displays an object selection window for one object. There may be NULL
 * elements in the linked list, corresponding to empty slots from the inventory
 * or a container.
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

            mvprintw(i + 1, 0, "%c - a %s", slot_to_letter(non_null_i),
                     object_name((Object *) cur->element));
            if (cur->element == rodney.wielded)
                printw(" (wielded)");
            if (cur->element == rodney.helm ||
                cur->element == rodney.body_armor)
                printw(" (being worn)");
            clrtoeol();

            cur = cur->next;
            non_null_i++;
            cur = find_not_null(cur, &non_null_i);
        }
        next_top = cur;

        if (next_top != NULL)
            mvprintw(getcury(stdscr) + 1, 0,
                     "<space> for next page, - to cancel");
        else
            mvprintw(getcury(stdscr) + 1, 0, "- to cancel");
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
            line_displayed = 0;
        }
    }

    return selected;
}

/*
 * print_to_log: print a message to the log file.
 */
void print_to_log(char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
}