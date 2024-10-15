#include "windowen.h"
#include <ncurses.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

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
        obj->window.obj = newwin(size_y, size_x, pos_y, pos_x);
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

    /*
    * strlen = 11
    * size_x = 10
    * real size_x = 8
    * x = 2
    * | hello w|rld
    * | hell...|
    * real_size_x - (x - 1) 
    *   
    */
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

void windowen_input(windowen* obj, int input) {
    if ( !obj ) {
        return;
    }

    switch ( input ) {
        case KEY_MOUSE:
            {
                MEVENT event = {0};
                if ( getmouse(&event) == OK ) {
                    if ( obj->mouse.function ) {
                        obj->mouse.function(obj->mouse.argument, event);
                    }
                }
            }

        break;

        default:
            {
                if ( obj->input.function ) {
                    obj->input.function(obj->input.argument, input);
                }
            }
        break;
    }

}

void windowen_delete(windowen* obj) {
    if (!obj) {
        return;
    }

    delwin(obj->window.obj);
    free(obj);
}

