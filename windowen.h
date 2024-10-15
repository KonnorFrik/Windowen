/** @file
 * @brief Window enhanced for ncurses
 *
 * Add input and draw callbacks with args and some function for manage a type
 */
#ifndef __NCURSES_GUI_MODULE__
#define __NCURSES_GUI_MODULE__

#include <ncurses.h>

typedef void(*callback_input)(void*, int);
typedef void(*callback_mouse)(void*, MEVENT);
typedef void(*callback_draw)(void*);
typedef void(*callback_update)(void*);

typedef struct {
    struct {
        callback_draw function;
        void* argument;
    } draw;

    struct {
        callback_update function;
        void* argument;
    } update;

    struct {
        callback_input function;
        void* argument;
    } input;

    struct {
        callback_mouse function;
        void* argument;
    } mouse;

    struct {
        WINDOW* obj;
        int size_x, size_y;
    } window;

    // TODO: add enum and function for turn on/off borders
    // add custom borders
    // add attrs for visible or not window
    //      also modify function for drawing with new attrs
} windowen;

/** @brief Create a new windowen
 */
windowen* windowen_new(int size_x, int size_y, int pos_x, int pos_y);

/** @brief Register given func as input processing
 */
windowen* windowen_register_input_callback(windowen* obj, callback_input func, void* args);

/** @brief Register given func as mouse input processing
 */
windowen* windowen_register_mouse_callback(windowen* obj, callback_mouse func, void* args);

/** @brief Register given func as draw processing
 */
windowen* windowen_register_draw_callback(windowen* obj, callback_draw func, void* args);

/** @brief Register given func as update processing
 */
windowen* windowen_register_update_callback(windowen* obj, callback_update func, void* args);

/** @brief Smart put string in bounds of window
 */
void windowen_addstr(windowen* obj, int x, int y, const char* str);

/** @brief Safe call registered update function
 */
void windowen_update(windowen* obj);

/** @brief Safe call registered draw function
 *
 * For first clear window and call box(..., 0, 0) for window
 * At last call refresh for window
 */
void windowen_draw(windowen* obj);

/** @brief Safe call registered input processing 
 */
void windowen_input(windowen* obj, int input);

/** @breif Delete windowen object
*
* Do not free callback arg
*
* @param[in, out] obj windowen object @see windowen
*/
void windowen_delete(windowen* obj);

#endif /* __NCURSES_GUI_MODULE__ */
