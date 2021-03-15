#include <ncurses.h>

void my_startcolor()
{
    start_color(); //enable color in terminal for pretty printing

    use_default_colors();

    //color inits
    // init_pair(1, COLOR_WHITE, COLOR_BLACK);
    // init_pair(2, COLOR_YELLOW, COLOR_BLUE);
    // init_pair(3, COLOR_BLUE, COLOR_GREEN);
    // init_pair(4, COLOR_MAGENTA, COLOR_WHITE);
    // init_pair(5, COLOR_GREEN, COLOR_RED);

    init_pair(1, COLOR_WHITE, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_BLUE,   -1);
    init_pair(4, COLOR_MAGENTA,-1);
    init_pair(5, COLOR_GREEN,  -1);
    init_pair(6, COLOR_CYAN,  -1);
    init_pair(7, COLOR_RED,  -1);
}
