#include "interface.h"
#include <stdio.h>
#include <time.h>
#include "instinct.h"
#include "file.h"
#include "main.h"
#include "display.h"
#include "audio.h"

void tape_button_pressed(Tape * tape, int button);
void tape_interface_update(Tape * tape, unsigned int tmillis, bool blink);
void tape_display_update(Tape * tape, int pixel_start, bool blink);
int check_button_held(int button, unsigned int tmillis);

void tape_jog(Tape * tape, int value);
void tape_seek(Tape * tape, int value);

int draw_graph(int pixel_start, int length, float value);
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
    case BTN_UP:
        audio_flush();
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
    case CTL_PITCH_DA:
        if (link_tapes) {
            tape_seek(&tape_a, value);
            tape_seek(&tape_b, value);
        } else {
            tape_seek(&tape_a, value);
        }
        break;
    case CTL_PITCH_DB:
        if (link_tapes) {
            tape_seek(&tape_a, value);
            tape_seek(&tape_b, value);
        } else {
            tape_seek(&tape_b, value);
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
        if (!tape->is_playing) {
            if (link_tapes)
                tape_a.record = tape_b.record = false;
            else
                tape->record = false;
        }
        break;
    case BTN_DECK_CUE:
        tape->record = !tape->record;
        break;
    case BTN_DECK_LISTEN:
        tape->aux_send = !tape->aux_send;
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
    default:
        if (button - tape->buttons_start <= BTN_DECK_CUE_KP4) {
            if (save_tape(tape))
                beep();
            tape->tape_num = button - tape->buttons_start;
            if (load_tape(tape))
                beep();
        }
    }
}

void interface_update(void) {
    unsigned int tmillis = time_millis();
    bool blink = tmillis % (BLINK_RATE * 2) >= BLINK_RATE;

    tape_interface_update(&tape_a, tmillis, blink);
    tape_interface_update(&tape_b, tmillis, blink);
    tape_display_update(&tape_a, 0, blink);
    tape_display_update(&tape_b, DISPLAY_LENGTH * 2, blink);

    set_led(BTN_SCRATCH, link_tapes);
}

void tape_interface_update(Tape * tape, unsigned int tmillis, bool blink) {
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
    set_led(btn_start + BTN_DECK_LISTEN, tape->aux_send);

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

    for (int i = 0; i < TAPE_LIBRARY_SIZE; i++)
        set_led(btn_start + i, i == tape->tape_num);
}

void tape_display_update(Tape * tape, int pixel_start, bool blink) {
    sample * head = tape->pt_head;
    sample * start = tape->pt_start;
    sample * end = tape->pt_end;
    sample * in = tape->pt_in;
    sample * out = tape->pt_out;

    draw_graph(pixel_start, DISPLAY_LENGTH - 1,
        (float)(head - start) / (end - start));
    set_pixel(pixel_start + DISPLAY_LENGTH - 1, head == end);

    if (head >= in && head <= out) {
        draw_graph(pixel_start + DISPLAY_LENGTH + 1, DISPLAY_LENGTH - 2,
            (float)(head - in) / (out - in));
        set_pixel(pixel_start + DISPLAY_LENGTH, true);
        set_pixel(pixel_start + DISPLAY_LENGTH * 2 - 1, head == out);
    } else {
        draw_graph(pixel_start + DISPLAY_LENGTH, DISPLAY_LENGTH - 1, 0);
        set_pixel(pixel_start + DISPLAY_LENGTH * 2 - 1, head > out);
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
    tape->pt_head += value * JOG_FRAMES * TAPE_CHANNELS;
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

void tape_seek(Tape * tape, int value) {
    tape_jog(tape, value == 1 ? 128 - SEEK_SPEED : SEEK_SPEED);
}

void draw_level(float level) {
    draw_graph(DISPLAY_LENGTH * 4, DISPLAY_LENGTH * 2, level);
}

int draw_graph(int pixel_start, int length, float value) {
    for (int i = 0; i < length; i++)
        set_pixel(pixel_start + i, i < value * length);
}

float control_to_volume(int control) {
    return linear_volume_to_exponential((float)control / 127.0);
}

