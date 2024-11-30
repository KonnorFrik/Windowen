#include <ncurses.h>
#include <stdlib.h>

#define DEBUG 0
#define VERBOSE 1

#if VERBOSE == 1
#include <stdlib.h>
#endif

#include "../windowen.h"

typedef struct {
    WINDOW* ret_on_click; // return this window on btn click
} button_arg;

void* on_button_hello_click(void* arg);
void* on_button_world_click(void* arg);

void window_hello_draw(void* arg);
void window_world_draw(void* arg);

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
#endif

    windowen* window_hello = windowen_new(10, 5, COLS / 2 - 5, LINES / 2);
    windowen_register_draw_callback(window_hello, window_hello_draw, window_hello);

    windowen* window_world = windowen_new(10, 5, COLS / 2 - 5, LINES / 2);
    windowen_register_draw_callback(window_world, window_world_draw, window_world);

    button_arg button_hello_arg = {
        .ret_on_click = (void*)window_hello,
    };
    button_arg button_world_arg = {
        .ret_on_click = (void*)window_world,
    };
    winenbtn_params button_hello_params = {
        .text = "hello",
        .size_x = 7,
        .size_y = 3,
        .position_x = COLS / 2 - 7,
        .position_y = 5,
        .is_borders = true,
        .mouse_button = BUTTON1_CLICKED,
    };
    winenbtn_params button_world_params = {
        .text = "world",
        .size_x = 7,
        .size_y = 3,
        .position_x = COLS / 2 + 7,
        .position_y = 5,
        .is_borders = true,
        .mouse_button = BUTTON1_CLICKED,
    };

    winenbtn* button_hello = winenbtn_new(button_hello_params);
    winenbtn_register_action(button_hello, on_button_hello_click, &button_hello_arg);

    winenbtn* button_world = winenbtn_new(button_world_params);
    winenbtn_register_action(button_world, on_button_world_click, &button_world_arg);

    bool loop = true;
    windowen* in_focus = NULL;

    while ( loop ) {
        winen_input input = windowen_getinput();

        switch ( input.input ) {
            case 'q':
                loop = false;
            break;

            case KEY_MOUSE:
                switch ( input.mouse_event.bstate ) {
                    case BUTTON1_CLICKED:
                        #if VERBOSE == 1
                        fprintf(stderr, "Mouse btn 1 cliced\n");
                        #endif
                    break;
                }
            break;
        }

        void* tmp = NULL;

        if ( (tmp = winenbtn_input(button_hello, input)) != NULL ) {
            in_focus = tmp;

            #if VERBOSE == 1
            fprintf(stderr, "Button hello clicked, in_focus = @%p\n", (void*)in_focus);
            #endif
        }

        if ( (tmp = winenbtn_input(button_world, input)) != NULL ) {
            in_focus = tmp;

            #if VERBOSE == 1
            fprintf(stderr, "Button world clicked, in_focus = @%p\n", (void*)in_focus);
            #endif
        }

        refresh();

        if ( in_focus ) {
            windowen_draw(in_focus);
        }

        winenbtn_draw(button_hello);
        winenbtn_draw(button_world);
    }

    winenbtn_delete(button_hello);
    winenbtn_delete(button_world);
#if DEBUG == 0
    printf("\033[?1003l\n"); // disable report ALL mouse events
    endwin();
#endif
    return 0;
}

void* on_button_hello_click(void* arg) {
    return ((button_arg*)arg)->ret_on_click;
}

void* on_button_world_click(void* arg) {
    return ((button_arg*)arg)->ret_on_click;
}

void window_hello_draw(void* arg) {
    windowen* self = arg;
    box(self->window.obj, 0, 0);

    mvwprintw(self->window.obj, self->window.size_y / 2, self->window.size_x / 2 - 3, "Hello");

    wrefresh(self->window.obj);
}

void window_world_draw(void* arg) {
    windowen* self = arg;
    box(self->window.obj, 0, 0);

    mvwprintw(self->window.obj, self->window.size_y / 2, self->window.size_x / 2 - 3, "World");

    wrefresh(self->window.obj);
}
