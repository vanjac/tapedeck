#include "interface.h"
#include <time.h>
#include "instinct.h"
#include "tape.h"
#include "main.h"

void tape_button_pressed(Tape * tape, int button);
void tape_interface_update(Tape * tape, int tmillis, bool blink);
int check_button_held(int button, int tmillis);

void tape_jog(Tape * tape, int value);

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
        tape_a.volume = (float)value / 127.0;
        break;
    case CTL_VOL_DB:
        tape_b.volume = (float)value / 127.0;
        break;
    case CTL_XFADER:
        audio_in_volume = (float)value / 127.0;
        break;
    case CTL_JOG_DA:
    case CTL_JOG_PRESSED_DA:
        tape_jog(&tape_a, value);
        break;
    case CTL_JOG_DB:
    case CTL_JOG_PRESSED_DB:
        tape_jog(&tape_b, value);
        break;
    }
}

void tape_button_pressed(Tape * tape, int button) {
    switch (button - tape->buttons_start) {
// playback/recording
    case BTN_DECK_PLAY:
        tape->is_playing = !tape->is_playing;
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
    int tmillis = time_millis();
    bool blink = tmillis % (BLINK_RATE * 2) >= BLINK_RATE;

    tape_interface_update(&tape_a, tmillis, blink);
    tape_interface_update(&tape_b, tmillis, blink);

    set_led(BTN_SCRATCH, link_tapes);
}

void tape_interface_update(Tape * tape, int tmillis, bool blink) {
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
}

int check_button_held(int button, int tmillis) {
    if (button_presses[button] && tmillis - button_presses[button] > HOLD_TIME) {
        button_presses[button] = 0;
        return 1;
    }
    return 0;
}

void tape_jog(Tape * tape, int value) {
    if (value >= 64)
        value -= 128;
    tape->jog_flag = true;
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

