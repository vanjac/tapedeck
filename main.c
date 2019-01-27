#include "main.h"
#include <stdio.h>
#include <sys/time.h>
#include "tape.h"
#include "instinct.h"
#include "audio.h"
#include "interface.h"

void mix(uint8_t * in1, uint8_t * in2, uint8_t * in3, uint8_t * out,
         bool enable1, bool enable2, bool enable3,
         float vol1, float vol2, float vol3);
void beep_sample(uint8_t * out);

// read/write a two-byte sample
// the byte at ptr and the byte after it will be used
short read_sample(uint8_t * ptr);
void write_sample(uint8_t * ptr, short sample);

int main(int argc, char *argv[]) {
    if (instinct_open())
        return 1;
    if (audio_open())
        return 1;
    if (tape_init(&tape_a))
        return 1;
    if (tape_init(&tape_b))
        return 1;
    tape_a.buttons_start = BTNS_DECK_A;
    tape_b.buttons_start = BTNS_DECK_B;

    audio_in_volume = 1.0;

    uint8_t audio_in_buffer[BUFFER_SIZE];
    uint8_t tape_a_out_buffer[BUFFER_SIZE];
    uint8_t tape_b_out_buffer[BUFFER_SIZE];
    uint8_t mix_buffer[BUFFER_SIZE];

    while(!quit_flag) {
        if (instinct_update())
            break;
        interface_update();

        if (audio_read(audio_in_buffer))
            break;
        int a_play = tape_playback(&tape_a, tape_a_out_buffer);
        int b_play = tape_playback(&tape_b, tape_b_out_buffer);

        // mix recording
        mix(tape_a_out_buffer, tape_b_out_buffer, audio_in_buffer, mix_buffer,
            a_play && tape_a.loopback, b_play && tape_b.loopback, true,
            tape_a.volume, tape_b.volume, audio_in_volume);
        tape_record(&tape_a, mix_buffer);
        tape_record(&tape_b, mix_buffer);

        if (beep_time >= 0)
            beep_sample(mix_buffer);
        else
            // mix playback
            mix(tape_a_out_buffer, tape_b_out_buffer, audio_in_buffer, mix_buffer,
                a_play, b_play, true,
                tape_a.volume, tape_b.volume, audio_in_volume);

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

short read_sample(uint8_t * ptr) {
    return *(ptr + 1) << 8 | *ptr; // little endian
}

void write_sample(uint8_t * ptr, short sample) {
    // little endian
    *(ptr + 1) = (sample >> 8) & 0xFF;
    *ptr = sample & 0xFF;
}

// depends on sample format being PA_SAMPLE_S16LE
// Signed 16 bit, little endian
void mix(uint8_t * in1, uint8_t * in2, uint8_t * in3, uint8_t * out,
         bool enable1, bool enable2, bool enable3,
         float vol1, float vol2, float vol3) {
    for (int i = 0; i < BUFFER_SIZE; i += 2) {
        int mixed = 0;
        if (enable1)
            mixed += read_sample(in1 + i) * vol1;
        if (enable2)
            mixed += read_sample(in2 + i) * vol2;
        if (enable3)
            mixed += read_sample(in3 + i) * vol3;
        if (mixed > 32767)
            mixed = 32767;
        else if (mixed < -32768)
            mixed = -32768;
        write_sample(out + i, mixed);
    }
}

int time_millis(void) {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

void beep(void) {
    beep_time = 0;
}

void beep_sample(uint8_t * out) {
    for (int i = 0; i < BUFFER_SIZE; i += 4) {
        short sample = beep_time * 1486; // should be about 1000 Hz
        sample /= 6;
        write_sample(out + i, sample);
        write_sample(out + i + 2, sample);
        beep_time++;
    }
    if (beep_time >= 4096)
        beep_time = -1;
}
