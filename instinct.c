#include "instinct.h"
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "interface.h"

#define MIDI_MESSAGE_SIZE 3

int midi_fd;

void interpret_midi_message(unsigned char b1, unsigned char b2, unsigned char b3);
void all_leds_off(void);

int instinct_open(void) {
    DIR *midi_dir = opendir(MIDI_DEVICE_DIR);
    if (midi_dir == NULL) {
        fprintf(stderr, "Couldn't open midi device directory %s\n", MIDI_DEVICE_DIR);
        return 1;
    }

    struct dirent *midi_dir_entry;
    char * midi_device_name = NULL;
    while ((midi_dir_entry = readdir(midi_dir)) != NULL) {
        char * name = midi_dir_entry->d_name;

        int match = 1;
        for (int i = 0; i < MIDI_DEVICE_PRE_LEN; i++) {
            if (name[i] != MIDI_DEVICE_PRE[i]) {
                match = 0;
                break;
            }
        }
        if (match) {
            midi_device_name = name;
        }
    }
    if (midi_device_name == NULL) {
        fprintf(stderr, "Couldn't find a midi device\n");
        return 1;
    }

    char midi_device_path[MIDI_DEVICE_PATH_MAX];
    sprintf(midi_device_path, "%s%s", MIDI_DEVICE_DIR, midi_device_name);
    printf("Found a midi device %s\n", midi_device_path);

    midi_fd = open(midi_device_path, O_RDWR | O_NONBLOCK);
    if (midi_fd == -1) {
        fprintf(stderr, "Couldn't open midi device\n");
        return 1;
    }

    all_leds_off();
    return 0;
}

void instinct_close(void) {
    all_leds_off();
    close(midi_fd);
}

int instinct_update(void) {
    unsigned char midi_message[MIDI_MESSAGE_SIZE];
    ssize_t midi_bytes_read;

    while(1) {
        midi_bytes_read = read(midi_fd, &midi_message, MIDI_MESSAGE_SIZE);
        if (midi_bytes_read < 0) { // read error
            if (errno == EWOULDBLOCK)
                break; // nothing to read
            else {
                fprintf(stderr, "MIDI read error! %d\n", errno);
                return 1;
            }
        }
        else if (midi_bytes_read > 0) {
            interpret_midi_message(midi_message[0], midi_message[1], midi_message[2]);
        }
    }
    return 0;
}

void interpret_midi_message(unsigned char b1, unsigned char b2, unsigned char b3) {
    if (b1 == 144) { // note event
        if (b3 >= 64) { // note on
            button_presses[b2] = clock();
            button_pressed(b2);
        } else {
            button_presses[b2] = 0;
        }
    } else if (b1 == 176) { // control event
        control_values[b2] = b3;
        control_changed(b2, b3);
    }
}

void set_led(int led, bool state) {
    unsigned char midi_message[MIDI_MESSAGE_SIZE];
    midi_message[0] = 144; // note event
    midi_message[1] = led;
    midi_message[2] = state ? 127 : 0;
    write(midi_fd, &midi_message, MIDI_MESSAGE_SIZE);
}

void all_leds_off(void) {
    for (int i = 1; i < NUM_BUTTONS; i++) {
        set_led(i, false);
        sleep(0.01);
    }
}

