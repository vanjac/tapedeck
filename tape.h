#include <stdbool.h>

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
    bool record, mute, loopback;
    float speed;
    OutPointAction out_point_action;
    float volume;
} Tape;

