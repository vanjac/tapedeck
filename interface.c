#include "interface.h"
#include <stdio.h>
#include "instinct.h"
#include "tape.h"

void tape_button_pressed(Tape * tape, int deck_button, int button);
void tape_control_changed(Tape * tape, int button, int value);

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
}

void control_changed(int control, int value) {
    printf("Control %d set to %d\n", control, value);
}

void tape_button_pressed(Tape * tape, int deck_button, int button) {
    switch (deck_button) {
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
    case BTN_DECK_LOAD:
        printf("Tape loopback toggle\n");
        tape->loopback = !tape->loopback;
        set_led(button, tape->loopback);
        break;
    case BTN_DECK_PREV:
        printf("Tape jump to start\n");
        tape->pt_head = tape->audio_data;
        break;
    }
}

int interface_update(void) {
    return 0;
}

