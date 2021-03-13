#ifndef _GUI_H_
#define _GUI_H_
#include <ncurses.h>

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void clear_msg_window(WINDOW *msg_win, char *username);
void print_to_chat(WINDOW *chat_win, char *name, char *msg, int *chat_line_counter, int usercolor);
#endif
