/* some paint
 * 
 * 'q' - exit
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

#define MOUSE_MOVE 268435456

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
        /*case 'h':*/
        /*    obj->cursor_x--;*/
        /**/
        /*    if ( obj->cursor_x < 1 ) {*/
        /*        obj->cursor_x = 1;*/
        /*    }*/
        /*break;*/
        /**/
        /*case 'j':*/
        /*    obj->cursor_y++;*/
        /**/
        /*    if ( obj->cursor_y >= obj->size_y ) {*/
        /*        obj->cursor_y = obj->size_y - 1;*/
        /*    }*/
        /*break;*/
        /**/
        /*case 'k':*/
        /*    obj->cursor_y--;*/
        /**/
        /*    if ( obj->cursor_y < 1 ) {*/
        /*        obj->cursor_y = 1;*/
        /*    }*/
        /*break;*/
        /**/
        /*case 'l':*/
        /*    obj->cursor_x++;*/
        /**/
        /*    if ( obj->cursor_x >= obj->size_x ) {*/
        /*        obj->cursor_x = obj->size_x - 1;*/
        /*    }*/
        /*break;*/

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
    box(obj->self->window.obj, 0, 0);

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
        move(LINES - 1, 1);
        printw("|Brush: '%c'|", obj->spawn_char);
}

void info_draw(void* arg) {
    info_t* obj = (info_t*)arg;
    wmove(obj->self->window.obj, 0, 0);
    wclrtobot(obj->self->window.obj);
    box(obj->self->window.obj, 0, 0);

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

void on_save_button_click(void* arg) {
    field* fld = (field*)arg;
    const char* filename = "art1.txt";
    FILE* file = fopen(filename, "wt");
    if ( !file ) {
        return;
    }

    for (int y = 0; y < fld->size_y; ++y) {
        for (int x = 0; x < fld->size_x; ++x) {
            if ( fld->field[y][x] ) {
                fprintf(file, "%c", fld->field[y][x]);
            } else {
                fprintf(file, "%c", ' ');
            }
        }

        fprintf(file, "%c", '\n');
    }

    fclose(file);
}

int main() {
    const int loop_delay = 100;
    #if DEBUG == 0
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    timeout(1);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    printf("\033[?12l");
    printf("\033[?1003h\n"); // enable report ALL mouse events
    #endif /* DEBUG == 0 */

    /*size_t sx = (COLS / 3) * 2, sy = LINES;*/
    size_t sx = 82, sy = 27;
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

    info_t info = {
        .field = &main_field,
    };

    windowen* info_window = windowen_new(COLS - 81, LINES / 2, sx, 0);
    info.self = info_window;
    windowen_register_draw_callback(info_window, info_draw, &info);

    winen_button_params save_btn_params = {
        .text = "Save",
        .size_x = 4,
        .size_y = 3,
        .position_x = info_window->window.position_x,
        .position_y = info_window->window.position_y + info_window->window.size_y + 1,
    };
    winen_button* save_button = winenbtn_new(save_btn_params);
    winenbtn_register_action(save_button, on_save_button_click, &main_field);

    int loop = 1;
    in_focus = main_windowen;
    winen_input input = {0};
    int cursor_x = 0, cursor_y = 0;

    while ( loop ) {
        input = windowen_getinput();

        // unhandled input processing
        switch ( input.input ) {
            case 'q':
                loop = 0;
            break;

            /*case KEY_MOUSE:*/
            /*    switch ( input.mouse_event.bstate ) {*/
            /*        case MOUSE_MOVE:*/
            /*            cursor_x = input.mouse_event.x;*/
            /*            cursor_y = input.mouse_event.y;*/
            /*        break;*/
            /*    }*/
            /*break;*/

            case ERR:
            default:
            break;
        }

        // safe call registered input processing
        windowen_input(in_focus, input);
        winenbtn_update(save_button, input);

        // safe call registered update
        windowen_update(main_windowen);
        /*windowen_update(log_window);*/

        // safe call registered draw
        windowen_draw(main_windowen);
        windowen_draw(info_window);
        winenbtn_draw(save_button);
        flushinp();
    }
    
    printf("\033[?1003l\n"); // disable report ALL mouse events
    #if DEBUG == 0
    endwin();
    #endif
    matrix_delete(main_field.field, main_field.size_y);
    return 0;
}
