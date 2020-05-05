#include "tape.h"
#include <stdlib.h>
#include <string.h>
#include "main.h"

int tape_init(Tape * tape) {
    tape->audio_data = malloc(TAPE_SAMPLES * sizeof(sample));
    if (!(tape->audio_data))
        return 1;

    tape_reset(tape);
    return 0;
}

void tape_reset(Tape * tape) {
    tape->pt_start = tape->audio_data + (TAPE_SAMPLES / 4);
    tape->pt_end = tape->pt_start;
    tape->pt_in = tape->pt_start;
    tape->pt_out = tape->pt_start;
    tape->pt_head = tape->pt_start;

    tape->is_playing = false;
    tape->record = false;
    tape->loopback = false;
    tape->out_point_action = OUT_CONTINUE;
    tape->volume = 1.0;
    tape->jog_flag = 0;
}

void tape_destroy(Tape * tape) {
    free(tape->audio_data);
}

int tape_playback(Tape * tape, sample * out_buffer, int num_samples) {
    if (!(tape->is_playing || tape->jog_flag)) {
        return 0;
    }
    if (tape->pt_head >= tape->pt_end) {
        // could happen while recording and expanding tape
        return 0;
    }

    for (int i = 0; i < num_samples; i++)
        out_buffer[i] = *(tape->pt_head + i);
    return 1;
}

void tape_record(Tape * tape, sample * in_buffer, int num_samples) {
    if (!(tape->is_playing && tape->record))
        return;
    if (tape->pt_head >= tape->pt_end) {
        if (tape->pt_head > TAPE_MAX(tape)) {
            // stop at end of audio data
            tape->pt_head = TAPE_MAX(tape);
            tape->is_playing = false;
            beep();
        } else {
            // extend tape. will be recorded to so no need to erase
            tape->pt_end = tape->pt_head + num_samples;
        }
    }
    for (int i = 0; i < num_samples; i++)
        *(tape->pt_head + i) = in_buffer[i];
}

void tape_move(Tape * tape, int num_samples) {
    if (tape->is_playing) {
        tape->pt_head += num_samples;

        if (tape->pt_head - num_samples < tape->pt_out
            && tape->pt_head >= tape->pt_out) {
            // if passed out point, do out point action
            switch(tape->out_point_action) {
            case OUT_CONTINUE:
                break;
            case OUT_LOOP:
                tape->pt_head = tape->pt_in;
                break;
            case OUT_STOP:
                tape->pt_head = tape->pt_out;
                tape->is_playing = false;
                break;
            }
        }

        if ((tape->pt_head >= tape->pt_end) && !(tape->record)) {
            // stop at end of tape
            tape->pt_head = tape->pt_end;
            tape->is_playing = false;
        }
    }

    if (tape->jog_flag)
        tape->jog_flag--;
}

int tape_expand(Tape * tape) {
    int ret = 0;
    if (tape->pt_head < tape->audio_data) {
        tape->pt_head = tape->audio_data;
        ret = 1;
    } else if (tape->pt_head > TAPE_MAX(tape)) {
        tape->pt_head = TAPE_MAX(tape);
        ret = 1;
    }

    if (tape->pt_head < tape->pt_start) {
        memset(tape->pt_head, 0, tape->pt_start - tape->pt_head);
        tape->pt_start = tape->pt_head;
    } else if (tape->pt_head > tape->pt_end) {
        memset(tape->pt_end, 0, tape->pt_head - tape->pt_end);
        tape->pt_end = tape->pt_head;
    }

    return ret;
}

void move_all_tape_points(Tape * tape, int offset) {
    tape->pt_head += offset;
    tape->pt_start += offset;
    tape->pt_end += offset;
    tape->pt_in += offset;
    tape->pt_out += offset;
}
