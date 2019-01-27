#define DISPLAY_LENGTH 24
#define DISPLAY_ROWS 2

#define COLOR_BLACK 0x00
#define COLOR_RED 0x01
#define COLOR_GREEN 0x02
#define COLOR_YELLOW 0x03

int display_open(void);
void display_close(void);
void set_pixel(int num, unsigned char value);
void display_update(void);

unsigned char display_pixels[DISPLAY_LENGTH * DISPLAY_ROWS];

