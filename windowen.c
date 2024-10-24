#include "windowen.h"

#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1

#if DEBUG == 1
#include <stdio.h>
#endif

windowen* windowen_new(int size_x, int size_y, int pos_x, int pos_y) {
    windowen* obj = calloc(1, sizeof(windowen));

    // NOTE:
    // In case it use box(..., 0, 0) function
    // min size = 3 for at least one cell without border
    if ( size_x < 3 ) {
        size_x = 3;
    }

    if ( size_y < 3 ) {
        size_y = 3;
    }

    if (obj) {
        obj->window.size_x = size_x;
        obj->window.size_y = size_y;
        obj->window.position_x = pos_x;
        obj->window.position_y = pos_y;
        obj->window.obj = newwin(size_y, size_x, pos_y, pos_x);

        // obj->settings.is_visible = 1;
    }

    return obj;
}

windowen* windowen_register_input_callback(windowen* obj, callback_input func, void* args) {
    if (!obj) {
        return obj;
    }

    obj->input.function = func;
    obj->input.argument = args;

    return obj;
}

windowen* windowen_register_mouse_callback(windowen* obj, callback_mouse func, void* args) {
    if (!obj) {
        return obj;
    }

    obj->mouse.function = func;
    obj->mouse.argument = args;

    return obj;
}

windowen* windowen_register_draw_callback(windowen* obj, callback_draw func, void* args) {
    if (!obj) {
        return obj;
    }

    obj->draw.function = func;
    obj->draw.argument = args;

    return obj;
}

windowen* windowen_register_update_callback(windowen* obj, callback_update func, void* args) {
    if (!obj) {
        return obj;
    }

    obj->update.function = func;
    obj->update.argument = args;

    return obj;
}

winen_input windowen_getinput() {
    int input = getch();
    MEVENT mouse_event = {0};
    bool is_mouse_valid = getmouse(&mouse_event);

    winen_input obj = {
        .input = input,
        .mouse_event = mouse_event,
        .is_mouse_valid = is_mouse_valid,
    };

    return obj;
}

// TODO: rewrite func - change windowen* to WINDOW* and max_size_x/y
void windowen_addstr(windowen* obj, int x, int y, const char* str) {
    if ( !obj ) {
        return;
    }

    // NOTE:
    // In case it use box(..., 0, 0) function 
    // Max x = size - 2 (2 - for draw borders)
    // Mix x = 1 (because borders draws from 0,0 of window)
    if ( x > (obj->window.size_x - 2) ) {
        x = obj->window.size_x - 2;
    }

    if ( y > (obj->window.size_y - 2) ) {
        y = obj->window.size_y - 2;
    }

    if ( x == 0 ) {
        x = 1;
    }

    if ( y == 0 ) {
        y = 1;
    }

    size_t len = strlen(str);
    char buf[len + 1];
    strncpy(buf, str, len);
    buf[len] = 0;

    size_t can_fit_from_x = ((obj->window.size_x - 2) - (x - 1));
    if ( len > can_fit_from_x && can_fit_from_x ) {
        buf[can_fit_from_x--] = 0;

        for (size_t count = 3; count && can_fit_from_x; --count) {
            buf[can_fit_from_x--] = '.';
        }
    }

    mvwaddstr(obj->window.obj, y, x, buf);
}

void windowen_update(windowen* obj) {
    if ( !obj ) {
        return;
    }

    if ( obj->update.function ) {
        obj->update.function(obj->update.argument);
    }
}

void windowen_draw(windowen* obj) {
    if ( !obj ) {
        return;
    }

    wclear(obj->window.obj);
    box(obj->window.obj, 0, 0);

    if ( obj->draw.function ) {
        obj->draw.function(obj->draw.argument);
    }

    wrefresh(obj->window.obj);
}

void windowen_input(windowen* obj, winen_input input) {
    if ( !obj ) {
        return;
    }

    switch ( input.input ) {
        case KEY_MOUSE:
            {
                if ( input.is_mouse_valid == OK ) {
                    if ( obj->mouse.function ) {
                        obj->mouse.function(obj->mouse.argument, input.mouse_event);
                    }
                }
            }

        break;

        default:
            {
                if ( obj->input.function ) {
                    obj->input.function(obj->input.argument, input.input);
                }
            }
        break;
    }

}

void windowen_delete(windowen* obj) {
    if ( !obj ) {
        return;
    }

    if ( obj->window.obj ) {
        delwin(obj->window.obj);
    }

    free(obj);
}

/* Possible ( and bad ) input: text
 * ""
 * "." delim = '.'
 * "hello.world."
*/
int popup_text(popup_params params) {
    if ( !params.text || !isascii(params.delimiter) ) {
        return 1;
    }

    char info_line[19] = {0};
    sprintf(info_line, "Quit:%c Up:%c Down:%c", params.key_close, params.key_scroll_up, params.key_scroll_down);
    // int status = 0;
    int max_win_size = 19; // max length of info line
    int text_len = 0;
    int rows = 0;
    const char* text_copy = params.text;

    if ( params.title ) {
        int len = strlen(params.title);

        if ( len > max_win_size && len < params.max_x ) {
            max_win_size = len;
        }

    } else {
        params.title = "";
    }

    if ( !text_copy ) {
        text_copy = "";
    }

    text_len = strlen(text_copy);

    if ( text_len ) {
        rows++;
    }

    char text_buffer[text_len + 1];
    strncpy(text_buffer, text_copy, text_len + 1);

    // parse text and create array
    for (int i = 0; i < text_len; ++i) {
        if ( text_buffer[i] == params.delimiter && i < (text_len - 1) ) {
            rows++;
        } 
    }

    char delim_str[2] = {0};
    sprintf(delim_str, "%c", params.delimiter);
    // init array with splitted text
    char* text_splitted[rows];
    memset(text_splitted, 0, rows);
    int was_delim = 1;
    int rows_writed = 0;

    for (int i = 0, row_start_ind = 0, row = 0; i < text_len; ++i) {
        if ( text_buffer[i] == params.delimiter ) {
            was_delim = 1;
            text_buffer[i] = 0;
            int row_len = i - row_start_ind;

            if ( row_len > max_win_size && row_len < params.max_x ) {
                max_win_size = row_len;
            }

        } else if ( isascii(text_buffer[i]) && was_delim ) {
            was_delim = 0;
            text_splitted[row++] = text_buffer + i;
            rows_writed++;
            row_start_ind = i;
        } 
    }

    if ( (max_win_size + 2) < params.max_x ) {
        max_win_size += 2; // 2 for borders
    }

    #if DEBUG == 1
    fprintf(stderr, "Title: '%s'\n", params.title);
    fprintf(stderr, "Text arg: '%s'\n", params.text);
    fprintf(stderr, "Text len: %d\n", text_len);
    fprintf(stderr, "Rows: %d\n", rows_writed);
    fprintf(stderr, "Win size: %d\n", max_win_size);
    fprintf(stderr, "Splitted text:\n");
    for (int i = 0; i < rows_writed; ++i) {
        fprintf(stderr, "%d - '%s'\n", i, text_splitted[i]);
    }
    #endif

    // create window 
    WINDOW* win = newwin(params.max_y, max_win_size, 0, params.max_x / 2 - max_win_size / 2);

    int input = 0;
    int title_x = max_win_size / 2 - strlen(params.title) / 2;
    int start_ind = 0;
    int end_ind = rows_writed - 1;
    int lines = params.max_y - 2;

    #if DEBUG == 1
    fprintf(stderr, "Title x: '%d'\n", title_x);
    #endif

    // show text and process input
    while ( input != params.key_close ) {
        int y = 1, x = 1;
        wclear(win);
        box(win, 0, 0);
        mvwaddstr(win, 0, title_x, params.title);
        mvwaddstr(win, params.max_y - 1, 1, info_line);

        if ( input == params.key_scroll_up ) {
            start_ind++;

        } else if ( input == params.key_scroll_down ) {
            start_ind--;
        }

        if ( start_ind >= rows_writed ) {
            start_ind = rows_writed - 1;
        }

        if ( start_ind < 0 ) {
            start_ind = 0;
        }
        
        end_ind = start_ind + lines;

        if ( end_ind >= rows_writed ) {
            end_ind = rows_writed - 1;
        }

        #if DEBUG == 1
        fprintf(stderr, "Writed: %d\n", rows_writed);
        fprintf(stderr, "Start : %d\n", start_ind);
        fprintf(stderr, "End   : %d\n", end_ind);
        #endif

        for (int i = start_ind; i <= end_ind; ++i) {
            mvwaddstr(win, y++, x, text_splitted[i]);
        }

        refresh();
        wrefresh(win);

        input = getch();
    }


    return 0;
}
