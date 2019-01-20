#include "instinct.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "interface.h"

// https://www.alsa-project.org/main/index.php/ALSA_Library_API
#define MIDI_DEVICE "/dev/snd/midiC1D0"

int midi_fd;

void interpret_midi_message(unsigned char b1, unsigned char b2, unsigned char b3);

int instinct_open(void) {
    midi_fd = open(MIDI_DEVICE, O_RDONLY | O_NONBLOCK);
    if (midi_fd == -1) {
        fprintf(stderr, "Couldn't open midi device %s\n", MIDI_DEVICE);
        return 1;
    }
    return 0;
}

void instinct_close(void) {
    close(midi_fd);
}

int instinct_update(void) {
    unsigned char midi_message[3];
    ssize_t midi_bytes_read;

    while(1) {
        midi_bytes_read = read(midi_fd, &midi_message, sizeof(midi_message));
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
            button_released(b2);
        }
    } else if (b1 == 176) { // control event
        control_values[b2] = b3;
        control_changed(b2, b3);
    }
}
