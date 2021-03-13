#ifndef _CHAT_REFRESH_THREAD_ARG_H_
#define _CHAT_REFRESH_THREAD_ARG_H_
#include <ncurses.h>

struct chat_refresh_thread_arg {
    WINDOW *chat_win;
    int sock;
    int chat_line_counter;
};

#endif
