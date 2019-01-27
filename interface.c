#include "interface.h"
#include <stdio.h>
#include <time.h>
#include "instinct.h"
#include "tape.h"
#include "main.h"
#include "display.h"

void tape_button_pressed(Tape * tape, int button);
void tape_interface_update(Tape * tape, unsigned int tmillis, bool blink,
    int pixel_start);
int check_button_held(int button, unsigned int tmillis);

void tape_jog(Tape * tape, int value);

int draw_point(uint8_t * position, uint8_t * cur_pos, uint8_t * next_pos);
float control_to_volume(int control);

void button_pressed(int button) {
    if (button >= BTNS_DECK_A && button < BTNS_DECK_A + NUM_DECK_NOTES) {
        tape_button_pressed(&tape_a, button);
        return;
    }
    if (button >= BTNS_DECK_B && button < BTNS_DECK_B + NUM_DECK_NOTES) {
        tape_button_pressed(&tape_b, button);
        return;
    }

    switch (button) {
    case BTN_SCRATCH:
        link_tapes = !link_tapes;
        break;
    case BTN_DOWN:
        quit_flag = true;
        break;
    }
}

void control_changed(int control, int value) {
    switch (control) {
    case CTL_VOL_DA:
        tape_a.volume = control_to_volume(value);
        break;
    case CTL_VOL_DB:
        tape_b.volume = control_to_volume(value);
        break;
    case CTL_XFADER:
        audio_in_volume = control_to_volume(value);
        break;
    case CTL_JOG_DA:
    case CTL_JOG_PRESSED_DA:
        if (link_tapes) {
            tape_jog(&tape_a, value);
            tape_jog(&tape_b, value);
        } else {
            tape_jog(&tape_a, value);
        }
        break;
    case CTL_JOG_DB:
    case CTL_JOG_PRESSED_DB:
        if (link_tapes) {
            tape_jog(&tape_a, value);
            tape_jog(&tape_b, value);
        } else {
            tape_jog(&tape_b, value);
        }
        break;
    }
}

void tape_button_pressed(Tape * tape, int button) {
    switch (button - tape->buttons_start) {
// playback/recording
    case BTN_DECK_PLAY:
        tape->is_playing = !tape->is_playing;
        if (link_tapes)
            tape_a.is_playing = tape_b.is_playing = tape->is_playing;
        break;
    case BTN_DECK_CUE:
        tape->record = !tape->record;
        break;
    case BTN_DECK_LISTEN:
        tape->loopback = !tape->loopback;
        break;
// navigation
    case BTN_DECK_PREV:
        if (tape->pt_head > tape->pt_out)
            tape->pt_head = tape->pt_out;
        else if (tape->pt_head > tape->pt_in)
            tape->pt_head = tape->pt_in;
        else
            tape->pt_head = tape->pt_start;
        break;
    case BTN_DECK_NEXT:
        if (tape->pt_head < tape->pt_in)
            tape->pt_head = tape->pt_in;
        else if (tape->pt_head < tape->pt_out)
            tape->pt_head = tape->pt_out;
        else
            tape->pt_head = tape->pt_end;
        break;
    case BTN_DECK_PBM:
        tape->pt_in = tape->pt_head;
        break;
    case BTN_DECK_PBP:
        tape->pt_out = tape->pt_head;
        break;
// other
    case BTN_DECK_SYNC:
        switch (tape->out_point_action) {
        case OUT_CONTINUE:
            tape->out_point_action = OUT_LOOP;
            break;
        case OUT_LOOP:
            tape->out_point_action = OUT_STOP;
            break;
        default:
            tape->out_point_action = OUT_CONTINUE;
        }
        break;
    }
}

void interface_update(void) {
    unsigned int tmillis = time_millis();
    bool blink = tmillis % (BLINK_RATE * 2) >= BLINK_RATE;

    tape_interface_update(&tape_a, tmillis, blink, 0);
    tape_interface_update(&tape_b, tmillis, blink, DISPLAY_LENGTH / 2);

    set_led(BTN_SCRATCH, link_tapes);
}

void tape_interface_update(Tape * tape, unsigned int tmillis, bool blink,
        int pixel_start) {
    int btn_start = tape->buttons_start;

    if (check_button_held(btn_start + BTN_DECK_PBM, tmillis)) {
        // set start of tape
        tape->pt_start = tape->pt_head;
        if (tape->pt_in < tape->pt_start)
            tape->pt_in = tape->pt_start;
        if (tape->pt_out < tape->pt_start)
            tape->pt_out = tape->pt_start;
        beep();
    }
    if (check_button_held(btn_start + BTN_DECK_PBP, tmillis)) {
        // set end of tape
        tape->pt_end = tape->pt_head;
        if (tape->pt_in > tape->pt_end)
            tape->pt_in = tape->pt_end;
        if (tape->pt_out > tape->pt_end)
            tape->pt_out = tape->pt_end;
        beep();
    }

    set_led(btn_start + BTN_DECK_PLAY, tape->is_playing);
    set_led(btn_start + BTN_DECK_CUE, tape->record);
    set_led(btn_start + BTN_DECK_LISTEN, tape->loopback);

    switch(tape->out_point_action) {
    case OUT_CONTINUE:
        set_led(btn_start + BTN_DECK_SYNC, false);
        break;
    case OUT_LOOP:
        set_led(btn_start + BTN_DECK_SYNC, true);
        break;
    case OUT_STOP:
        set_led(btn_start + BTN_DECK_SYNC, blink);
        break;
    }

    set_led(btn_start + BTN_DECK_LOOP_KP1, tape->pt_head == tape->pt_start);
    set_led(btn_start + BTN_DECK_LOOP_KP2, tape->pt_head == tape->pt_in);
    set_led(btn_start + BTN_DECK_LOOP_KP3, tape->pt_head == tape->pt_out);
    set_led(btn_start + BTN_DECK_LOOP_KP4, tape->pt_head == tape->pt_end);

    // draw tape
    long tape_len = tape->pt_end - tape->pt_start;
    long pixel_len = DISPLAY_LENGTH / 2 - 1;
    for (long i = 0; i < pixel_len + 1; i++) {
        uint8_t * cur_pos = tape_len * i / pixel_len + tape->pt_start;
        uint8_t * next_pos = tape_len * (i + 1) / pixel_len + tape->pt_start;

        unsigned char color = COLOR_BLACK;
        if (draw_point(tape->pt_start, cur_pos, next_pos)
            || draw_point(tape->pt_in, cur_pos, next_pos)
            || draw_point(tape->pt_out, cur_pos, next_pos)
            || draw_point(tape->pt_end, cur_pos, next_pos))
            color |= COLOR_RED;
        if (draw_point(tape->pt_head, cur_pos, next_pos))
            color |= COLOR_GREEN;
        set_pixel(i + pixel_start, color);
    }
}

int check_button_held(int button, unsigned int tmillis) {
    if (button_presses[button] && tmillis - button_presses[button] > HOLD_TIME) {
        button_presses[button] = 0;
        return 1;
    }
    return 0;
}

void tape_jog(Tape * tape, int value) {
    if (value >= 64)
        value -= 128;
    tape->jog_flag = JOG_REPEAT_COUNT;
    tape->pt_head += value * BUFFER_SIZE;
    if (tape->record) {
        if (tape_expand(tape))
            beep();
    } else {
        if (tape->pt_head <= tape->pt_start)
            tape->pt_head = tape->pt_start;
        else if (tape->pt_head >= tape->pt_end)
            tape->pt_head = tape->pt_end;
    }
}


int draw_point(uint8_t * position, uint8_t * cur_pos, uint8_t * next_pos) {
    return position >= cur_pos && position < next_pos;
}

float control_to_volume(int control) {
    return linear_volume_to_exponential((float)control / 127.0);
}

void draw_level(float level) {
    for (int i = 0; i < DISPLAY_LENGTH; i++) {
        if (i < level * DISPLAY_LENGTH)
            set_pixel(i + DISPLAY_LENGTH, COLOR_RED);
        else
            set_pixel(i + DISPLAY_LENGTH, COLOR_BLACK);
    }
}
