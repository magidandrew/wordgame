#include <ncurses.h>
#include <string.h>

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win = newwin(height, width, starty, startx);
//    box(local_win, 0, 0); /* 0, 0 gives default characters
//					 * for the vertical and horizontal
//					 * lines			*/
    wrefresh(local_win);  /* Show that box*/

    return local_win;
}

void destroy_win(WINDOW *local_win)
{
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    /* 1. win: the window on which to operate
     * 2. ls: character to be used for the left side of the window
     * 3. rs: character to be used for the right side of the window
     * 4. ts: character to be used for the top side of the window
     * 5. bs: character to be used for the bottom side of the window
     * 6. tl: character to be used for the top left corner of the window
     * 7. tr: character to be used for the top right corner of the window
     * 8. bl: character to be used for the bottom left corner of the window
     * 9. br: character to be used for the bottom right corner of the window*/
    wrefresh(local_win);
    delwin(local_win);
}

void clear_msg_window(WINDOW *msg_win, char *username)
{
    wmove(msg_win, 1, strlen(username) + 2);
    wclrtoeol(msg_win);
    wrefresh(msg_win);
}

void print_to_chat(WINDOW *chat_win, char *name, char *msg, int *chat_line_counter, int usercolor)
{
    wattron(chat_win, COLOR_PAIR(usercolor));
//    mvwprintw(chat_win, *chat_line_counter, 1, "%s:", name);
    wprintw(chat_win, "%s:", name);
    wattroff(chat_win, COLOR_PAIR(usercolor));

//    mvwprintw(chat_win, *chat_line_counter, 2 + strlen(name), "%s", msg);
    wprintw(chat_win, "%s\n", msg);
    wrefresh(chat_win);
    (*chat_line_counter)++;
}

