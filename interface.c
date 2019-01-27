#include "interface.h"
#include <stdio.h>
#include "instinct.h"
#include "tape.h"
#include "main.h"

void tape_button_pressed(Tape * tape, int button);
void tape_interface_update(Tape * tape);

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
    printf("Control %d set to %d\n", control, value);
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
        tape->pt_head = tape->pt_start;
        break;
    case BTN_DECK_NEXT:
        tape->pt_head = tape->pt_end;
        break;
    case BTN_DECK_PBM:
        tape->pt_in = tape->pt_head;
        break;
    case BTN_DECK_PBP:
        tape->pt_out = tape->pt_head;
        break;
    }
}

void interface_update(void) {
    tape_interface_update(&tape_a);
    tape_interface_update(&tape_b);

    set_led(BTN_SCRATCH, link_tapes);
}

void tape_interface_update(Tape * tape) {
    int led_start = tape->buttons_start;
    set_led(led_start + BTN_DECK_PLAY, tape->is_playing);
    set_led(led_start + BTN_DECK_CUE, tape->record);
    set_led(led_start + BTN_DECK_LISTEN, tape->loopback);

    set_led(led_start + BTN_DECK_LOOP_KP1, tape->pt_head == tape->pt_start);
    set_led(led_start + BTN_DECK_LOOP_KP2, tape->pt_head == tape->pt_in);
    set_led(led_start + BTN_DECK_LOOP_KP3, tape->pt_head == tape->pt_out);
    set_led(led_start + BTN_DECK_LOOP_KP4, tape->pt_head == tape->pt_end);
}

void tape_jog(Tape * tape, int value) {
    if (value >= 64)
        value -= 128;
    tape->jog_flag = true;
    tape->pt_head += value * BUFFER_SIZE;
    if (tape->pt_head <= tape->pt_start)
        tape->pt_head = tape->pt_start;
    if (tape->pt_head >= tape->pt_end)
        tape->pt_head = tape->pt_end;
}

