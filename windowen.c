#include "windowen.h"

#include <ncurses.h>
#include <stdlib.h>
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
        obj->window.position_x = pos_x;
        obj->window.position_y = pos_y;
        obj->window.obj = newwin(size_y, size_x, pos_y, pos_x);

        obj->settings.is_visible = 1;
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

void windowen_show(windowen* obj) {
    if ( !obj ) {
        return;
    }

    obj->settings.is_visible = 1;
}

void windowen_hide(windowen* obj) {
    if ( !obj ) {
        return;
    }

    obj->settings.is_visible = 0;
    wclear(obj->window.obj);
}

int windowen_isvisible(windowen* obj) {
    if ( !obj ) {
        return 0;
    }

    return obj->settings.is_visible;
}

void windowen_update(windowen* obj) {
    if ( !obj || !obj->settings.is_visible ) {
        return;
    }

    if ( obj->update.function ) {
        obj->update.function(obj->update.argument);
    }
}

void windowen_draw(windowen* obj) {
    if ( !obj || !obj->settings.is_visible ) {
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

    delwin(obj->window.obj);
    free(obj);
}

