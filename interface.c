#include "interface.h"
#include <stdio.h>
#include "instinct.h"
#include "tape.h"
#include "main.h"

void tape_button_pressed(Tape * tape, int deck_button, int button);
void tape_control_changed(Tape * tape, int button, int value);
void tape_interface_update(Tape * tape, int led_start);

void button_pressed(int button) {
    if (button >= BTNS_DECK_A && button < BTNS_DECK_A + NUM_DECK_NOTES) {
        printf("Tape A note on %d\n", button);
        tape_button_pressed(&tape_a, button - BTNS_DECK_A, button);
        return;
    }
    if (button >= BTNS_DECK_B && button < BTNS_DECK_B + NUM_DECK_NOTES) {
        printf("Tape B note on %d\n", button);
        tape_button_pressed(&tape_b, button - BTNS_DECK_B, button);
        return;
    }
    printf("Other note on %d\n", button);
    switch (button) {
    case BTN_SCRATCH:
        printf("Link toggle\n");
        link_tapes = !link_tapes;
        set_led(button, link_tapes);
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
        printf("Tape A volume\n");
        tape_a.volume = (float)value / 127.0;
        break;
    case CTL_VOL_DB:
        printf("Tape B volume\n");
        tape_b.volume = (float)value / 127.0;
        break;
    case CTL_XFADER:
        printf("Audio in volume\n");
        audio_in_volume = (float)value / 127.0;
    }
}

void tape_button_pressed(Tape * tape, int deck_button, int button) {
    switch (deck_button) {
// playback/recording
    case BTN_DECK_PLAY:
        printf("Tape play/pause\n");
        tape->is_playing = !tape->is_playing;
        set_led(button, tape->is_playing);
        break;
    case BTN_DECK_CUE:
        printf("Tape record toggle\n");
        tape->record = !tape->record;
        set_led(button, tape->record);
        break;
    case BTN_DECK_LISTEN:
        printf("Tape loopback toggle\n");
        tape->loopback = !tape->loopback;
        set_led(button, tape->loopback);
        break;
// navigation
    case BTN_DECK_PREV:
        printf("Tape jump to start\n");
        tape->pt_head = tape->pt_start;
        break;
    case BTN_DECK_NEXT:
        printf("Tape jump to end\n");
        tape->pt_head = tape->pt_end;
        break;
    case BTN_DECK_PBM:
        printf("Tape set in point\n");
        tape->pt_in = tape->pt_head;
        break;
    case BTN_DECK_PBP:
        printf("Tape set out point\n");
        tape->pt_out = tape->pt_head;
        break;
    }
}

void interface_update(void) {
    tape_interface_update(&tape_a, BTNS_DECK_A);
    tape_interface_update(&tape_b, BTNS_DECK_B);
}

void tape_interface_update(Tape * tape, int led_start) {
    set_led(led_start + BTN_DECK_LOOP_KP1, tape->pt_head == tape->pt_start);
    set_led(led_start + BTN_DECK_LOOP_KP2, tape->pt_head == tape->pt_in);
    set_led(led_start + BTN_DECK_LOOP_KP3, tape->pt_head == tape->pt_out);
    set_led(led_start + BTN_DECK_LOOP_KP4, tape->pt_head == tape->pt_end);
}

