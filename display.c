#include "display.h"
#include <ncurses.h>

void force_set_pixel(int num, unsigned char value);
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

void set_pixel(int num, unsigned char value) {
    if (display_pixels[num] != value)
        force_set_pixel(num, value);
}

void force_set_pixel(int num, unsigned char value) {
    display_pixels[num] = value;

    char c;
    switch (value) {
    case COLOR_BLACK:
        c = '.';
        break;
    case COLOR_RED:
        c = '-';
        break;
    case COLOR_GREEN:
        c = '|';
        break;
    case COLOR_YELLOW:
        c = '+';
        break;
    }
    int row = num / DISPLAY_LENGTH;
    int col = num % DISPLAY_LENGTH;
    mvprintw(row, col, "%c", c);
}

void all_pixels_off(void) {
    for (int i = 0; i < DISPLAY_LENGTH * DISPLAY_ROWS; i++)
        force_set_pixel(i, COLOR_BLACK);
}
