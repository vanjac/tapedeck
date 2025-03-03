#pragma once

#define BLINK_RATE 256
#define HOLD_TIME 700
#define JOG_FRAMES BUFFER_FRAMES
#define JOG_REPEAT_COUNT 2
#define SEEK_SPEED 16

void button_pressed(int button);
void control_changed(int control, int value);
void interface_update(void);
void draw_level(float level);
