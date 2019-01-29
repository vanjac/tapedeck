#include "display.h"
#include <ncurses.h>

void force_set_pixel(int num, bool value);
void all_pixels_off(void);

int display_open(void) {
    initscr();
    curs_set(0); // hide cursor
    all_pixels_off();
    return 0;
}

void display_close(void) {
    all_pixels_off();
    endwin();
}

void display_update(void) {
    refresh();
}

void set_pixel(int num, bool value) {
    if (display_pixels[num] != value)
        force_set_pixel(num, value);
}

void force_set_pixel(int num, bool value) {
    display_pixels[num] = value;

    int led_panel_num = num / DISPLAY_LENGTH;

    int row, col;
    switch (led_panel_num) {
    case 0:
        col = 0;
        row = 0;
        break;
    case 1:
        col = 0;
        row = 1;
        break;
    case 2:
        col = DISPLAY_LENGTH + 2;
        row = 0;
        break;
    case 3:
        col = DISPLAY_LENGTH + 2;
        row = 1;
        break;
    case 4:
        col = 1;
        row = 3;
        break;
    case 5:
        col = DISPLAY_LENGTH + 1;
        row = 3;
        break;
    }
    col += num % DISPLAY_LENGTH;

    char c = value ? '#' : '.';
    mvprintw(row, col, "%c", c);
}

void all_pixels_off(void) {
    for (int i = 0; i < DISPLAY_LENGTH * DISPLAY_ROWS; i++)
        force_set_pixel(i, false);
}
