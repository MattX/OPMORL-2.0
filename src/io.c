/*
 *  io.c (was pline.c)
 *  OPMORL 2
 *
 *  Created by Théotime Grohens on 13/11/10.
 *  Copyright 2010-2016 OPMORL 2 dev team. All rights reserved.
 *
 */

#include <unistd.h>
#include "opmorl.h"

// Maximum height of full screen windows like inventory selection
#define WINDOW_HEIGHT 18


/**
 * Get one key of player input
 * @return The pressed key
 */
char get_input()
{
    return (char) getch();
    line_needs_confirm = false;
}


/**
 * Prints map, environment messages, and stats.
 */
void display_everything()
{
    display_stats(); /* The stuff like health points, level et alii */
    show_env_messages();
    if (!line_needs_confirm)
        clear_msg_line();
    display_map();
}


/**
 * Return the appropriate glyph (-, | or +) to represent
 * the wall at (level, x, y).
 */
char select_wall_glyph(int level, Coord pos)
{
    bool left_wall = false, right_wall = false, up_wall = false,
            down_wall = false;

    if ((pos.x != 0 && maps[level][pos.x - 1][pos.y] == T_WALL))
        up_wall = true;
    if ((pos.x != LEVEL_HEIGHT - 1 && maps[level][pos.x + 1][pos.y] == T_WALL))
        down_wall = true;

    if ((pos.y != 0 && maps[level][pos.x][pos.y - 1] == T_WALL))
        left_wall = true;
    if ((pos.y != LEVEL_WIDTH - 1 && maps[level][pos.x][pos.y + 1] == T_WALL))
        right_wall = true;

    if (up_wall && down_wall && !left_wall && !right_wall)
        return '|';
    if (!up_wall && !down_wall && left_wall && right_wall)
        return '-';

    return '+';
}


/**
 * Display a map of the current level.
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

            // TODO: display colors
            mvaddch(i + 1, j, tile_types[maps[rodney.dlvl][i][j]].sym);

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
            if (obj->dlvl != rodney.dlvl ||
                visibility_map[rodney.dlvl][obj->pos.x][obj->pos.y] ==
                TS_UNDISCOVERED)
                continue;

            attron(COLOR_PAIR(obj->type->color));
            mvaddch(obj->pos.x + 1, obj->pos.y, obj->type->class->symbol);
            attroff(COLOR_PAIR(obj->type->color));
        } while ((obj_node = obj_node->next) != NULL);

    /* Monsters */
    if (mon_node)
        do {
            Monster *mon = (Monster *) mon_node->element;
            if (mon->dlvl != rodney.dlvl ||
                visibility_map[rodney.dlvl][mon->pos.x][mon->pos.y] != TS_SEEN)
                continue;

            attron(COLOR_PAIR(magic_class_colors[mon->type->magic_class]));
            mvaddch(mon->pos.x + 1, mon->pos.y, mon->type->symbol);
            attroff(COLOR_PAIR(magic_class_colors[mon->type->magic_class]));
        } while ((mon_node = mon_node->next) != NULL);

    /* Rodney */
    attron(A_BOLD);
    mvaddch(rodney.pos.x + 1, rodney.pos.y, '@');
    attroff(A_BOLD);

    move(rodney.pos.x + 1, rodney.pos.y);
}

/**
 * Display player stats at the bottom of the screen.
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

/**
 * Clear the message line on top of the screen.
 */
void clear_msg_line()
{
    move(0, 0);
    clrtoeol();
}

/**
 * Print a line at the top of the screen. Supports printf variable arguments,
 * passed as a va_list. This is to allow calling from other functions.
 */
void va_pline(char *format, va_list args)
{
    int max_x, line_len;
    char buf[BUFSIZ];

    vsnprintf(buf, BUFSIZ - 1, format, args);
    max_x = getmaxx(stdscr);
    line_len = (int) strlen(buf);

    if (line_needs_confirm) {
        // Leave space for the --more-- prompt
        if (last_col + line_len + 11 < max_x) {
            mvprintw(0, last_col + 1, buf);
        } else {
            mvprintw(0, last_col + 1, " --more--");
            getch();
            line_needs_confirm = false;
        }
    }

    if (!line_needs_confirm) {
        move(0, 0);
        printw(buf);
    }

    line_needs_confirm = true;
    last_col = getcurx(stdscr);

    clrtoeol();
}

/**
 * Print a line at the top of the screen. Supports printf variable arguments.
 */
void pline(char *format, ...)
{
    va_list args;
    va_start(args, format);
    va_pline(format, args);
    va_end(args);
}

/**
 * Ask a yes-no question.
 */
bool yes_no(char *format, ...)
{
    char append[] = " [yn] ";
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
        rep = get_input();
        if (rep == 'y' || rep == 'Y')
            return true;
        if (rep == 'n' || rep == 'N')
            return false;
    }
}


/**
 * Asks to select a point on the map. This will return false if the user
 * cancelled point selection, true otherwise.
 * @param selected A pointer to where the selected point will be stored.
 */
bool get_point(Coord *selected)
{
    pline("Move cursor with movement keys. Use {.,: } to confirm, ESC to cancel.");
    line_needs_confirm = 0;

    Coord cur = rodney.pos;

    move(cur.x + 1, cur.y);
    curs_set(2);

    int status = 0; /* 0: selecting, 1: selected, 2: canceled */

    while (!status) {
        int c = getch();

        switch (c) {
        case '.':
        case ',':
        case ' ':
        case ':':
            status = 1;
            break;
        case 27: // Alt or esc
            if (getch() == -1) {
                status = 2;
            }
            break;
        case 'h':
        case 'j':
        case 'k':
        case 'l':
        case 'y':
        case 'u':
        case 'b':
        case 'n':
            cur = coord_add(cur, letter_to_direction((char) c));
            move(cur.x + 1, cur.y);
        default:
            break;
        }
    }

    curs_set(1);

    *selected = cur;

    return status == 1 ? true : false;
}


// TODO: object selection window for several objects

/**
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
            int selection = get_input();
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


/*
 * select_from_inv: Ask the player an object from the player inventory. Returns
 * NULL if the selection is cancelled.
 * TODO: allow restricting selection to some types only.
 */
Object *select_from_inv(int possible_types)
{
    LinkedList *inv = array_to_linked_list((void **) rodney.inventory,
                                           INVENTORY_SIZE, false);
    Object *selected = select_object(inv);
    delete_linked_list(inv);

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


/**
 * Shows the introduction text full-screen. Gets the file from INTRO_FILE.
 */
void show_intro()
{
    if (line_needs_confirm) {
        get_input();
    }

    erase();

    FILE *intro_file = fopen(INTRO_FILE, "r");
    if (intro_file == NULL) {
        pline("Could not open intro file");
        return;
    }

    char line_buf[BUFSIZ];

    for (int i = 0; i < getmaxy(stdscr); i++) {
        if (!fgets(line_buf, BUFSIZ, intro_file))
            break;
        mvprintw(i, 0, "%s", line_buf);
    }

    fclose(intro_file);

    get_input();
}

/**
 * Gets a direction from the player
 * @return a coord representing the direction.
 */
Coord get_direction()
{
    char dir = get_input();

    return letter_to_direction(dir);
}