#include "main.h"
#include <stdio.h>
#include "tape.h"
#include "instinct.h"
#include "audio.h"
#include "interface.h"

int main(int argc, char *argv[]) {
    if (instinct_open())
        return 1;
    if (audio_open())
        return 1;
    if (tape_init(&tape_a))
        return 1;
    if (tape_init(&tape_b))
        return 1;

    uint8_t audio_in_buffer[BUFFER_SIZE];
    uint8_t tape_a_out_buffer[BUFFER_SIZE];
    uint8_t tape_b_out_buffer[BUFFER_SIZE];
    uint8_t mix_buffer[BUFFER_SIZE];

    while(1) {
        if (instinct_update())
            break;
        if (interface_update())
            break;

        if (audio_read(audio_in_buffer))
            break;
        tape_playback(&tape_a, tape_a_out_buffer);
        tape_playback(&tape_b, tape_b_out_buffer);

        // TODO: mix recording
        for (int i = 0; i < BUFFER_SIZE; i++)
            mix_buffer[i] = audio_in_buffer[i];
        tape_record(&tape_a, mix_buffer);
        tape_record(&tape_b, mix_buffer);

        // TODO: mix audio out

        if (audio_write(mix_buffer))
            break;

        tape_move(&tape_a);
        tape_move(&tape_b);
    }

    printf("Quit\n");
    instinct_close();
    audio_close();
    tape_destroy(&tape_a);
    tape_destroy(&tape_b);
    return 0;
}

