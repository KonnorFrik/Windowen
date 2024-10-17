/* some physics
 * 
 * 'q' - exit
 * 'h' 'j' 'k' 'l' - move
 */

#define _DEFAULT_SOURCE
#include "../windowen.h"
#include <ncurses.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#define DEBUG 0
#define VERBOSE 0

static windowen* in_focus = NULL;

typedef struct {
    windowen* self;
    winen_input* input;
} log_info;

typedef struct {
    windowen* self;
    int** field;
    int size_x, size_y;
    int cursor_x, cursor_y;
    int is_spawn;
    int spawn_char;

    struct {
        int drawed_elems;
    } info;
} field;

typedef struct {
    windowen* self;
    field* field;
} info_t;


int** matrix_new(int size_x, int size_y) {
    int** matrix = calloc(size_y, sizeof(int*));

    for (int i = 0; i < size_y; ++i) {
        int* row = calloc(size_x, sizeof(int));
        matrix[i] = row;
    }

    return matrix;
}

void matrix_delete(int** matrix, int size_y) {
    if ( !matrix ) {
        return;
    }

    for (int i = 0; i < size_y; ++i) {
        if ( matrix[i] ) {
            free(matrix[i]);
        }
    }

    free(matrix);
}

double current_time_millis() {
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double)(tv.tv_sec) * 1000 + (double)(tv.tv_usec) / 1000;
}

void field_input(void* arg, int input) {
    field* obj = (field*)arg;
    
    switch ( input ) {
        case 'h':
            obj->cursor_x--;
            
            if ( obj->cursor_x < 1 ) {
                obj->cursor_x = 1;
            }
        break;

        case 'j':
            obj->cursor_y++;
            
            if ( obj->cursor_y >= obj->size_y ) {
                obj->cursor_y = obj->size_y - 1;
            }
        break;

        case 'k':
            obj->cursor_y--;
            
            if ( obj->cursor_y < 1 ) {
                obj->cursor_y = 1;
            }
        break;

        case 'l':
            obj->cursor_x++;
            
            if ( obj->cursor_x >= obj->size_x ) {
                obj->cursor_x = obj->size_x - 1;
            }
        break;

        case '\n':
            for (int r = 0; r < obj->size_y; ++r) {
                for (int c = 0; c < obj->size_x; ++c) {
                    obj->field[r][c] = 0;
                }
            }
            obj->info.drawed_elems = 0;
        break;

        case ERR:
        break;
            
        default:
            if ( isascii(input) ) {
                obj->spawn_char = input;
            }
        break;
    }
    // fprintf(stderr, "Field input: inp:%c\n", input);
}

void field_mouse(void* arg, MEVENT event) {
    field* obj = (field*)arg;

    if ( event.bstate & BUTTON1_CLICKED ) {
        obj->is_spawn = !obj->is_spawn;
        obj->cursor_x = event.x;
        obj->cursor_y = event.y;

    } else if ( event.bstate & BUTTON1_RELEASED ) {
        obj->is_spawn = 0;

    } else if ( event.bstate & REPORT_MOUSE_POSITION ) {
        // fprintf(stderr, "MOUSE POS REPORT\n");
        obj->cursor_x = event.x;
        obj->cursor_y = event.y;
    }
}

void field_update(void* arg) {
    field* obj = (field*)arg;

    if ( obj->is_spawn ) {
        #if VERBOSE == 1
        fprintf(stderr, "Spawn at X:%d Y:%d\n", obj->cursor_x, obj->cursor_y);
        #endif
        bool is_x_in_bounds = obj->cursor_x > 0 && obj->cursor_x < obj->size_x;
        bool is_y_in_bounds = obj->cursor_y > 0 && obj->cursor_y < obj->size_y;
        bool is_ascii = isascii(obj->field[obj->cursor_y][obj->cursor_x]);
        bool is_not_same = obj->field[obj->cursor_y][obj->cursor_x] != obj->spawn_char;
        bool is_valid_to_spawn = (obj->spawn_char != 0 || obj->spawn_char != -1);

        if ( is_x_in_bounds &&
             is_y_in_bounds && 
             is_ascii &&
             is_not_same &&
             is_valid_to_spawn ) {

            obj->field[obj->cursor_y][obj->cursor_x] = obj->spawn_char;
            obj->info.drawed_elems++;
        }
    }
}

void field_draw(void* arg) {
    field* obj = (field*)arg;

    for (int r = 0; r < obj->size_y; ++r) {
        for (int c = 0; c < obj->size_x; ++c) {
            if ( obj->field[r][c] ) {
                mvwaddch(obj->self->window.obj, r, c, obj->field[r][c]);

                #if VERBOSE == 1
                fprintf(stderr, "Draw at: X:%d Y:%d V:%c\n", c, r, obj->field[r][c]);
                #endif
            }
        }
    }
}

void info_draw(void* arg) {
    info_t* obj = (info_t*)arg;
    int max_x = getmaxx(obj->self->window.obj);
    int y = 1;
    int x = 1;
    windowen_addstr(obj->self, (max_x / 2) - 2, y++, "INFO");

    char buf[128] = {0};
    sprintf(buf, "Drawed elems: %d", obj->field->info.drawed_elems);
    windowen_addstr(obj->self, x, y++, buf);

    sprintf(buf, "Is draw: %d", obj->field->is_spawn);
    windowen_addstr(obj->self, x, y++, buf);

    sprintf(buf, "Char: %d", obj->field->spawn_char);
    windowen_addstr(obj->self, x, y++, buf);


    y = LINES - 2;
    windowen_addstr(obj->self, x, y--, "'q' - for exit");
    windowen_addstr(obj->self, x, y--, "Any char - select for drawing");
    windowen_addstr(obj->self, x, y--, "Mouse moving - drawing");
    windowen_addstr(obj->self, x, y--, "Click - turn on/off drawing");
}

void pop_up_draw(void* arg) {
    windowen* obj = (windowen*)arg;
    mvwaddch(obj->window.obj, 0, obj->window.size_x - 1, 'X');
    windowen_addstr(obj, 1, 1, "Hello)");
}

void pop_up_mouse(void* arg, MEVENT event) {
    windowen* obj = (windowen*)arg;

    if ( event.bstate & BUTTON3_CLICKED ) {
        windowen_show(obj);
        #if VERBOSE == 1
        fprintf(stderr, "POP UP is show\n");
        #endif
        // add move window

    } else if ( event.bstate & BUTTON1_CLICKED && windowen_isvisible(obj) ) {
        int x = event.x, y = event.y;
        if ( wmouse_trafo(obj->window.obj, &y, &x, FALSE) == TRUE &&
             (x == obj->window.size_x - 1 && y == 0) ) {
            windowen_hide(obj);

            #if VERBOSE == 1
            fprintf(stderr, "POP UP M1 at X:%d Y:%d\n", x, y);
            #endif
        }

    }
}

// void log_input(void* arg, int input) {
//     if ( input == ERR ) {
//         return;
//     }
//
//     windowen* obj = (windowen*)arg;
//     char buf[128] = {0};
//     snprintf(buf, 128, "Input: %s (%d)", keyname(input), input);
//
//     fprintf(stderr, "LOG WINDOW: '%s'\n", buf);
//
//     wmove(obj->window.obj, 0, 0);
//     winsertln(obj->window.obj);
//     waddstr(obj->window.obj, buf);
//     wclrtoeol(obj->window.obj);
// }
//
// void log_mouse(void* arg, MEVENT event) {
//     windowen* obj = (windowen*)arg;
//     char buf[128] = {0};
//     snprintf(buf, 128, "Mouse: at X:%d Y:%d (%08x)", event.x, event.y, event.bstate);
//
//     fprintf(stderr, "LOG WINDOW: '%s'\n", buf);
//
//     wmove(obj->window.obj, 0, 0);
//     winsertln(obj->window.obj);
//     waddstr(obj->window.obj, buf);
//     wclrtoeol(obj->window.obj);
// }

void log_draw(void* arg) {
    log_info* obj = (log_info*)arg;
    char buf[128] = {0};
    mvwaddstr(obj->self->window.obj, 0, obj->self->window.size_x / 2, "LOG");
    bool is_print = 0;

    switch ( obj->input->input ) {
        case KEY_MOUSE:
            snprintf(buf,
                     128,
                     "Mouse: at X:%d Y:%d (%08x)",
                     obj->input->mouse_event.x,
                     obj->input->mouse_event.y,
                     obj->input->mouse_event.bstate
                     );
            is_print = 1;
        break;

        default:
            snprintf(buf,
                     128,
                     "Input: %s (%d)",
                     keyname(obj->input->input),
                     obj->input->input
                     );
            is_print = 1;
        break;

        case 0:
        case ERR:
        break;
    }

    if ( is_print ) {
        // fprintf(stderr, "LOG WINDOW: '%s'\n", buf);

        wmove(obj->self->window.obj, 1, 1);
        winsertln(obj->self->window.obj);
        waddstr(obj->self->window.obj, buf);
        wclrtoeol(obj->self->window.obj);
        wrefresh(obj->self->window.obj);
    }
}

int main() {
    const int loop_delay = 100;
    #if DEBUG == 0
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    timeout(loop_delay);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    printf("\033[?1003h\n"); // enable report ALL mouse events
    #endif /* DEBUG == 0 */

    size_t sx = (COLS / 3) * 2, sy = LINES;
    windowen* main_windowen = windowen_new(sx, sy, 0, 0);
    field main_field = {
        .size_x = sx - 1,
        .size_y = sy - 1,
        .cursor_x = 1,
        .cursor_y = 1,
        .self = main_windowen,
    };
    main_field.field = matrix_new(main_field.size_x, main_field.size_x);
    windowen_register_input_callback(main_windowen, field_input, &main_field);
    windowen_register_mouse_callback(main_windowen, field_mouse, &main_field);
    windowen_register_draw_callback(main_windowen, field_draw, &main_field);
    windowen_register_update_callback(main_windowen, field_update, &main_field);

    info_t info = {
        .field = &main_field,
    };

    windowen* info_window = windowen_new((COLS / 3) * 1, LINES / 2, sx, 0);
    info.self = info_window;
    windowen_register_draw_callback(info_window, info_draw, &info);

    windowen* pop_up = windowen_new(10, 3, 5, 5);
    windowen_register_draw_callback(pop_up, pop_up_draw, pop_up);
    windowen_register_mouse_callback(pop_up, pop_up_mouse, pop_up);
    windowen_hide(pop_up);

    windowen* log_window = windowen_new((COLS / 3) * 1, LINES / 2, sx, LINES / 2);
    log_info loger = {.self = log_window, .input = 0};
    // windowen_register_input_callback(log_window, log_input, &loger);
    // windowen_register_mouse_callback(log_window, log_mouse, &loger);
    windowen_register_update_callback(log_window, log_draw, &loger);

    int loop = 1;
    in_focus = main_windowen;
    winen_input input = {0};
    loger.input = &input;

    while ( loop ) {
        double input_start = current_time_millis();
        input = windowen_getinput();
        double elapsed = current_time_millis() - input_start;

        // unhandled input processing
        switch ( input.input ) {
            case 'q':
                loop = 0;
            break;

            case KEY_MOUSE:
                if ( input.mouse_event.bstate & BUTTON3_CLICKED ) {
                    if ( !windowen_isvisible(pop_up) ) {
                        in_focus = pop_up;
                    }

                } else if ( input.mouse_event.bstate & BUTTON1_CLICKED ) {
                    if ( !windowen_isvisible(pop_up) ) {
                        in_focus = main_windowen;
                    }
                }
            break;

            case ERR:
            default:
            break;
        }

        // safe call registered input processing
        // windowen_input(main_windowen, input);
        // windowen_input(pop_up, input);
        windowen_input(in_focus, input);
        // windowen_input(log_window, input);

        // safe call registered update
        windowen_update(main_windowen);
        windowen_update(log_window);

        // safe call registered draw
        windowen_draw(main_windowen);
        windowen_draw(info_window);
        windowen_draw(pop_up);
        // windowen_draw(log_window);


        mvprintw(LINES - 2, 1, "input: %c(%d)", input.input, input.input);

        // special processing for mouse movement
        int target_delay = (loop_delay - elapsed) * 1000;

        if ( target_delay < 0 ) {
            target_delay = 0;
        }

        // fprintf(stderr, "target_delay: %d microSec\n", target_delay);
        usleep(target_delay);
        flushinp();
    }
    
    printf("\033[?1003l\n"); // disable report ALL mouse events
    #if DEBUG == 0
    endwin();
    #endif
    matrix_delete(main_field.field, main_field.size_y);
    return 0;
}
