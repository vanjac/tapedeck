#include <stdbool.h>
#include <pulse/simple.h>
#include "main.h"

// 6 minutes 20 seconds of audio
// with sample rate of 44100 Hz, 16 bit samples, 2 channels
#define TAPE_SIZE 67108864
#define TAPE_MAX(tape) tape->audio_data + TAPE_SIZE - BUFFER_SIZE
#define TAPE_LIBRARY_SIZE 16

typedef enum {OUT_CONTINUE, OUT_LOOP, OUT_STOP} OutPointAction;

typedef struct {
    // saved to file
    unsigned char tape_num;
    uint8_t *audio_data;
    // these should all be multiples of BUFFER_SIZE away from the start!
    uint8_t *pt_head, *pt_start, *pt_end, *pt_in, *pt_out;

    // state (not saved)
    bool is_playing;
    bool record, loopback;
    OutPointAction out_point_action;
    float volume;
    char invert; // 1 or -1

    unsigned char jog_flag; // number of times jog sample should be played

    unsigned short buttons_start; // start of MIDI notes for this tape
} Tape;

Tape tape_a, tape_b;

int tape_init(Tape * tape);
void tape_reset(Tape * tape);
void tape_destroy(Tape * tape);
// return false if tape isn't playing and nothing was written to out_buffer
int tape_playback(Tape * tape, uint8_t * out_buffer);
void tape_record(Tape * tape, uint8_t * in_buffer);
void tape_move(Tape * tape);
// resize tape in/out points to include pt_head
// return false if tape couldn't expand past audio memory
int tape_expand(Tape * tape);
void move_all_tape_points(Tape * tape, long offset);
