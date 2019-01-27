#include <stdbool.h>

#define DISPLAY_LENGTH 10
#define DISPLAY_ROWS 6

int display_open(void);
void display_close(void);
void set_pixel(int num, bool value);
void display_update(void);

bool display_pixels[DISPLAY_LENGTH * DISPLAY_ROWS];

