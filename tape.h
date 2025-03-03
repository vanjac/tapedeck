#pragma once

#include <stdbool.h>
#include "main.h"

// 6 minutes 20 seconds of audio
// with sample rate of 44100 Hz, 2 channels
#define TAPE_SAMPLES 33554432
#define TAPE_MAX(tape) tape->audio_data + TAPE_SAMPLES - BUFFER_SAMPLES
#define TAPE_LIBRARY_SIZE 16

typedef enum {OUT_CONTINUE, OUT_LOOP, OUT_STOP} OutPointAction;

typedef struct {
    // saved to file
    unsigned char tape_num;
    sample *audio_data;
    // these should all be multiples of BUFFER_SAMPLES away from the start!
    sample *pt_head, *pt_start, *pt_end, *pt_in, *pt_out;

    // state (not saved)
    bool is_playing;
    bool record, aux_send;
    OutPointAction out_point_action;
    float volume;

    unsigned char jog_flag; // number of times jog sample should be played

    unsigned short buttons_start; // start of MIDI notes for this tape
} Tape;

extern Tape tape_a, tape_b;

int tape_init(Tape * tape);
void tape_reset(Tape * tape);
void tape_destroy(Tape * tape);
// return false if tape isn't playing and nothing was written to out_buffer
int tape_playback(Tape * tape, sample * out_buffer, int num_samples);
void tape_record(Tape * tape, sample * in_buffer, int num_samples);
void tape_move(Tape * tape, int num_samples);
// resize tape in/out points to include pt_head
// return false if tape couldn't expand past audio memory
int tape_expand(Tape * tape);
void move_all_tape_points(Tape * tape, int offset);
