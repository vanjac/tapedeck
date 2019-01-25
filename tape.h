#include <stdbool.h>
#include <pulse/simple.h>

// 6 minutes 20 seconds of audio
// with sample rate of 44100 Hz, 16 bit samples, 2 channels
#define TAPE_SIZE 67108864

#define NUM_MARKS 4

typedef enum {OUT_CONTINUE, OUT_STOP, OUT_LOOP} OutPointAction;

typedef struct {
    // saved to file
    unsigned short num;
    uint8_t *audio_data;
    uint8_t *pt_head, *pt_start, *pt_end, *pt_in, *pt_out;
    uint8_t *marks[NUM_MARKS];

    // state (not saved)
    bool is_playing;
    bool record, loopback;
    OutPointAction out_point_action;
    float volume;

    bool jog_flag;
} Tape;

Tape tape_a, tape_b;

int tape_init(Tape * tape);
void tape_destroy(Tape * tape);
// return false if tape isn't playing and nothing was written to out_buffer
int tape_playback(Tape * tape, uint8_t * out_buffer);
void tape_record(Tape * tape, uint8_t * in_buffer);
void tape_move(Tape * tape);

