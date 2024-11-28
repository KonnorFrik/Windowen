#include "../windowen.h"
#include <ncurses.h>
#include <stdio.h>

#define DEBUG 0
#define MOUSE_MOVE 268435456

void on_b1_clicked(void* arg) {
    fprintf(stderr, "Button 1 clicked\n");
}

void on_b2_clicked(void* arg) {
    fprintf(stderr, "Button 2 clicked\n");
    if ( arg ) {
        *(int*)arg = 0;
    }
}

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

    winen_button_params b1_params = {
        .text = "click me",
        .size_x = 10,
        .size_y = 3,
        .position_x = 1,
        .position_y = 1,
        .is_borders = true,
    };
    winen_button* b1 = winenbtn_new(b1_params);
    if ( !b1 ) {
        goto exit;
    }
    winenbtn_register_action(b1, on_b1_clicked, NULL);

    int loop = 1;

    winen_button_params b2_params = {
        .text = "quit",
        .size_x = 4,
        .size_y = 1,
        .position_x = COLS - 4,
        .position_y = 1,
        .is_borders = false,
    };
    winen_button* b2 = winenbtn_new(b2_params);
    if ( !b2 ) {
        goto exit;
    }
    winenbtn_register_action(b2, on_b2_clicked, &loop);

    winen_input input = {0};
    int cursor_x = 0, cursor_y = 0;
    

    while ( loop ) {
        input = windowen_getinput();

        winenbtn_draw(b1);
        winenbtn_update(b1, input);

        winenbtn_draw(b2);
        winenbtn_update(b2, input);

        switch ( input.input ) {
            case 'q':
                loop = 0;
            break;

            case KEY_MOUSE:
                switch ( input.mouse_event.bstate ) {
                    case MOUSE_MOVE:
                        cursor_x = input.mouse_event.x;
                        cursor_y = input.mouse_event.y;
                    break;
            }
            break;
        }

        move(cursor_y, cursor_x);
    }

    winenbtn_delete(b1);
    winenbtn_delete(b2);
exit:
    #if DEBUG == 0
    printf("\033[?1003l\n"); // disable report ALL mouse events
    endwin();
    #endif /* DEBUG == 0 */
    fprintf(stderr, "EXIT\n");
    return 0;
}
