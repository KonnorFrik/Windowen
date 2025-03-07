/** @file
 * @brief Window enhanced for ncurses
 *
 * Enhance a WINDOW type from ncurses
 * Add new struct, callbacks and function for manage new struct
 */
#ifndef __NCURSES_GUI_MODULE__
#define __NCURSES_GUI_MODULE__

#include <ncurses.h>
///< Callback signature to register for input processing by window
typedef void*(*callback_input)(void*, int);
///< Callback signature to register for mouse input processing by window
typedef void*(*callback_mouse)(void*, MEVENT);
///< Callback signature to register for drawing
typedef void(*callback_draw)(void*);
///< Callback signature to register for update states
typedef void(*callback_update)(void*);

///< Callback signature to register for click button
typedef void*(*callback_button_click)(void*);
///< Callback signature to register draw function for button
typedef void(*callback_button_draw)(void*);

typedef struct {
    int max_y, max_x;
    int delimiter;
    const char* text;
    int key_scroll_up;
    int key_scroll_down;
    int key_close;
    const char* title;
} popup_params;

/** @brief Wrap for WINDOW object from ncurses
 */
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
        int position_x, position_y;
    } window;
} windowen;

/** @brief Wrap ncurses input (keyboard and mouse) for pass it to windowen_input functions
 */
typedef struct {
    int input;
    MEVENT mouse_event;
    bool is_mouse_valid;
} winen_input;

typedef struct {
    struct {
        callback_button_click function;
        void* argument;
    } action;

    struct {
        callback_button_draw function;
        void* argument;
    } draw;

    struct {
        WINDOW* obj;
        int size_x, size_y;
        int position_x, position_y;
    } window;

    const char* text;
    bool is_borders;
    int mouse_button;
} winenbtn;

/** @brief Hold parameters for create a winenbtn object */
typedef struct {
    int size_y, size_x; ///< button size
    int position_y, position_x; ///< button position
    char* text; ///< text for button (must not be freed while button is alive)
    bool is_borders; ///< draw borders for button (true) or not (false)
    int mouse_button; ///< what mouse event to trigger to
} winenbtn_params;

/** @brief Create a new button
 * @param params Parameters for new button
 * @return button pointer or NULL
 */
winenbtn* winenbtn_new(winenbtn_params params);

/** @brief Register function as callback when button clicked only with mouse
 * @param[in, out] self Pointer to valid winenbtn object
 * @param callback User function to call on click
 * @param arg Pointer to user argument for pass into callback
 */
void winenbtn_register_action(winenbtn* self, callback_button_click callback, void* arg);

/** @brief Register optional function for drawing
 *
 * If user drawing function registered - call it for draw button
 * use default implementation otherwise
 *
 * @param[in, out] self Pointer to valid winenbtn object
 * @param callback User function to call on draw
 * @param arg Pointer to user argument for pass into callback
 */
void winenbtn_register_draw(winenbtn* self, callback_button_draw callback, void* arg);

/** @brief Draw button on screen
 * @param[in, out] self Pointer to valid winenbtn object
 */
void winenbtn_draw(winenbtn* self);

/** @brief Handles whether a button was pressed
 * @param[in, out] self Pointer to valid winenbtn object
 * @param input Struct with input data @see windowen_getinput
 * @return user-defined arg
 */
void* winenbtn_input(winenbtn* self, winen_input input);

/** @brief Delete a button object
 * @param[in, out] self Pointer to valid winenbtn object
 */
void winenbtn_delete(winenbtn* self);


/** @brief Create a new windowen object
 * @param[in] size_x Size of new window for x
 * @param[in] size_y Size of new window for y
 * @param[in] pos_x  Position of new window for x
 * @param[in] pos_y  Position of new window for y
 * @return object New windowen object, NULL if error
 */
windowen* windowen_new(int size_x, int size_y, int pos_x, int pos_y);

/** @brief Register given func as input processing
 * @param[in, out] obj  Windowen object for register callback into it
 * @param[in]      func Callback function
 * @param[in]      args Arguments for pass them into callback
 * @return obj Same windowen object as input
 */
windowen* windowen_register_input_callback(windowen* obj, callback_input func, void* args);

/** @brief Register given func as mouse input processing
 * @param[in, out] obj  Windowen object for register callback into it
 * @param[in]      func Callback function
 * @param[in]      args Arguments for pass them into callback
 * @return obj Same windowen object as input
 */
windowen* windowen_register_mouse_callback(windowen* obj, callback_mouse func, void* args);

/** @brief Register given func as draw processing
 * @param[in, out] obj  Windowen object for register callback into it
 * @param[in]      func Callback function
 * @param[in]      args Arguments for pass them into callback
 * @return obj Same windowen object as input
 */
windowen* windowen_register_draw_callback(windowen* obj, callback_draw func, void* args);

/** @brief Get input from ncurses
 * @return obj The winen_input object
 */
winen_input windowen_getinput();

/** @brief Smart put string in bounds of window
 * @param[in, out] obj Windowen object for register callback into it
 * @param[in]      x   Position of x for given window
 * @param[in]      y   Position of y for given window
 * @param[in]      str String for draw
 */
void windowen_addstr(windowen* obj, int x, int y, const char* str);

/** @brief Safe call registered update function
 * @param[in, out] obj Windowen object with registered callback function
 */
/*void windowen_update(windowen* obj);*/

/** @brief Safe call registered draw function
 *
 * For first clear window and call box(..., 0, 0) for window
 * At last call refresh for window
 *
 * For invisible window call only wclear
 *
 * @param[in, out] obj Windowen object with registered callback function
 */
void windowen_draw(windowen* obj);

/** @brief Safe call registered input processing 
 * @param[in, out] obj   Windowen object with registered callback function
 * @param[in]      input input from ncurses
 * @return user-defined arg
 */
void* windowen_input(windowen* obj, winen_input input);

/** @breif Delete windowen object
*
* Do not free any callback args
*
* @param[in, out] obj windowen object for delete
*/
void windowen_delete(windowen* obj);

/** @brief Create a new window with max x or max len of one line and max y, show given text splitted
 *
 * Allow to scroll text
 *
 * @param params Struct with config
 * @return status 0 for OK, any otherwise mean error
 */
int popup_text(popup_params params);

#endif /* __NCURSES_GUI_MODULE__ */
