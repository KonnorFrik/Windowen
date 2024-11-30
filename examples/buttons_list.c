#include "../windowen.h"
#include <ncurses.h>
#include <stdio.h>

#define DEBUG 0
#define MOUSE_MOVE 268435456

void main_window_input(void* arg, int key);

/*
 * MENU:
 * keys for move inside window
 * text must be scrollable
 * each press on KEY_ACCEPT - must do something (ex: switch to sub-menu)
 */

int main() {
    #if DEBUG == 0
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    timeout(1);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    printf("\033[?1003h\n"); // enable report ALL mouse events
    #endif /* DEBUG == 0 */

    windowen* main_window = windowen_new(COLS / 2, LINES, 0, 0);
    windowen_register_input_callback(main_window, main_window_input, NULL);

    bool loop = true;

    while ( loop ) {
    }

    #if DEBUG == 0
    printf("\033[?1003l\n"); // disable report ALL mouse events
    endwin();
    #endif /* DEBUG == 0 */
    fprintf(stderr, "EXIT\n");
    return 0;
}

void main_window_input(void* arg, int key) {
}

