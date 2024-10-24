#include "../windowen.h"
#include <ncurses.h>

char* lorem_ipsum = "Lorem ipsum dolor sit amet, officia excepteur ex fugiat reprehenderit enim labore culpa sint ad nisi Lorem pariatur mollit ex esse exercitation amet. Nisi anim cupidatat excepteur officia. Reprehenderit nostrud nostrud ipsum Lorem est aliquip amet voluptate voluptate dolor minim nulla est proident. Nostrud officia pariatur ut officia. Sit irure elit esse ea nulla sunt ex occaecat reprehenderit commodo officia dolor Lorem duis laboris cupidatat officia voluptate. Culpa proident adipisicing id nulla nisi laboris ex in Lorem sunt duis officia eiusmod. Aliqua reprehenderit commodo ex non excepteur duis sunt velit enim. Voluptate laboris sint cupidatat ullamco ut ea consectetur et est culpa et culpa duis.";

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    popup_params params = {
        .title = "Lorem Ipsum",
        .key_close = 'q',
        .max_x = COLS,
        .max_y = LINES,
        .delimiter = '.',
        .text = lorem_ipsum,
        .key_scroll_up = 'w',
        .key_scroll_down = 's',
    };

    popup_text(params);

    endwin();

    return 0;
}
