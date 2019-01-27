#include "tape.h"
#include <stdlib.h>
#include <string.h>
#include "main.h"

int tape_init(Tape * tape) {
    tape->audio_data = malloc(TAPE_SIZE);
    if (!(tape->audio_data))
        return 1;

    tape->pt_start = tape->audio_data + (TAPE_SIZE / 4);
    tape->pt_end = tape->pt_start;
    tape->pt_in = tape->pt_start;
    tape->pt_out = tape->pt_start;
    tape->pt_head = tape->pt_start;

    tape->is_playing = false;
    tape->record = false;
    tape->loopback = false;
    tape->out_point_action = OUT_CONTINUE;
    tape->volume = 1.0;
    tape->jog_flag = false;
    return 0;
}

void tape_destroy(Tape * tape) {
    free(tape->audio_data);
}

int tape_playback(Tape * tape, uint8_t * out_buffer) {
    if (!(tape->is_playing || tape->jog_flag)) {
        return 0;
    }

    // TODO: volume
    for (int i = 0; i < BUFFER_SIZE; i++)
        out_buffer[i] = *(tape->pt_head + i);
    return 1;
}

void tape_record(Tape * tape, uint8_t * in_buffer) {
    if (!(tape->is_playing && tape->record))
        return;
    if (tape->pt_head >= tape->pt_end) {
        // extend tape. will be recorded to so no need to erase
        tape->pt_end = tape->pt_head + BUFFER_SIZE;
        // TODO: stop at end of audio data
    }
    for (int i = 0; i < BUFFER_SIZE; i++)
        *(tape->pt_head + i) = in_buffer[i];
}

void tape_move(Tape * tape) {
    if (tape->is_playing) {
        tape->pt_head += BUFFER_SIZE;

        if ((tape->pt_head >= tape->pt_end) && !(tape->record)) {
            tape->pt_head = tape->pt_end;
            tape->is_playing = false;
        }
        // TODO: out point action
    }

    tape->jog_flag = false;
}

