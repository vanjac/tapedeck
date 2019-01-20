#include "main.h"
#include <stdio.h>
#include "tape.h"
#include "instinct.h"
#include "audio.h"
#include "interface.h"

void mix(uint8_t * in1, uint8_t * in2, uint8_t * in3, uint8_t * out,
         bool enable1, bool enable2, bool enable3);
void mix_sample(uint8_t * in, int * out);

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

        // mix recording
        mix(tape_a_out_buffer, tape_b_out_buffer, audio_in_buffer,
            mix_buffer, tape_a.loopback, tape_b.loopback, true);
        tape_record(&tape_a, mix_buffer);
        tape_record(&tape_b, mix_buffer);

        // mix playback
        mix(tape_a_out_buffer, tape_b_out_buffer, audio_in_buffer,
            mix_buffer, !(tape_a.mute), !(tape_b.mute), true);

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

// depends on sample format being PA_SAMPLE_S16LE
// Signed 16 bit, little endian
void mix(uint8_t * in1, uint8_t * in2, uint8_t * in3, uint8_t * out,
         bool enable1, bool enable2, bool enable3) {
    for (int i = 0; i < BUFFER_SIZE; i += 2) {
        int mixed = 0;
        if (enable1)
            mix_sample(in1 + i, &mixed);
        if (enable2)
            mix_sample(in2 + i, &mixed);
        if (enable3)
            mix_sample(in3 + i, &mixed);
        if (mixed > 32767)
            mixed = 32767;
        else if (mixed < -32768)
            mixed = -32768;
        short mixed_short = mixed;
        // little endian
        out[i + 1] = (mixed_short >> 8) & 0xFF;
        out[i] = mixed_short & 0xFF;
    }
}

void mix_sample(uint8_t * in, int * out) {
    short in_value = *(in + 1) << 8 | *in; // little endian
    *out += in_value;
}
