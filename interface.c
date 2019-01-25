#include "interface.h"
#include <stdio.h>
#include "instinct.h"
#include "tape.h"
#include "main.h"

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
    switch (button) {
    case BTN_SCRATCH:
        printf("Link toggle\n");
        link_tapes = !link_tapes;
        set_led(button, link_tapes);
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
    case BTN_DECK_PREV:
        printf("Tape jump to start\n");
        tape->pt_head = tape->audio_data;
        break;
    }
}

int interface_update(void) {
    return 0;
}

