#include "main.h"
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include "file.h"
#include "instinct.h"
#include "audio.h"
#include "interface.h"
#include "display.h"

// return peak
float mix(sample * in1, sample * in2, sample * in3, sample * out, int num_samples,
         bool enable1, bool enable2, bool enable3,
         float vol1, float vol2, float vol3);
void beep_sample(sample * out, int num_samples);

int beep_time;

int main(int argc, char *argv[]) {
    if (instinct_open())
        return 1;
    if (audio_open())
        return 1;
    if (display_open())
        return 1;
    if (tape_init(&tape_a))
        return 1;
    if (tape_init(&tape_b))
        return 1;

    tape_a.buttons_start = BTNS_DECK_A;
    tape_b.buttons_start = BTNS_DECK_B;
    tape_a.tape_num = 8; // L1
    tape_b.tape_num = 9; // L2
    if (load_tape(&tape_a))
        beep();
    if (load_tape(&tape_b))
        beep();

    audio_in_volume = 1.0;

    sample audio_in_buffer[BUFFER_SAMPLES];
    sample tape_a_out_buffer[BUFFER_SAMPLES];
    sample tape_b_out_buffer[BUFFER_SAMPLES];
    sample mix_buffer[BUFFER_SAMPLES];

    while(!quit_flag) {
        if (instinct_update())
            break;
        interface_update();

        if (audio_read(audio_in_buffer, BUFFER_SAMPLES))
            break;
        int a_play = tape_playback(&tape_a, tape_a_out_buffer, BUFFER_SAMPLES);
        int b_play = tape_playback(&tape_b, tape_b_out_buffer, BUFFER_SAMPLES);

        // mix recording
        mix(audio_in_buffer, NULL, NULL, mix_buffer, BUFFER_SAMPLES,
            true, false, false,
            audio_in_volume, 0, 0);
        tape_record(&tape_a, mix_buffer, BUFFER_SAMPLES);
        tape_record(&tape_b, mix_buffer, BUFFER_SAMPLES);
        
        // mix aux send
        mix(tape_a_out_buffer, tape_b_out_buffer, NULL, mix_buffer, BUFFER_SAMPLES,
            a_play && tape_a.aux_send, b_play && tape_b.aux_send, false,
            tape_a.volume, tape_b.volume, 0);
        if (audio_write_aux(mix_buffer, BUFFER_SAMPLES))
            break;

        // mix playback
        float peak = mix(tape_a_out_buffer, tape_b_out_buffer, audio_in_buffer,
            mix_buffer, BUFFER_SAMPLES,
            a_play && !tape_a.aux_send, b_play && !tape_b.aux_send, true,
            tape_a.volume, tape_b.volume, audio_in_volume);
        if (beep_time)
            beep_sample(mix_buffer, BUFFER_SAMPLES);

        draw_level(exponential_volume_to_linear(peak));

        if (audio_write(mix_buffer, BUFFER_SAMPLES))
            break;

        tape_move(&tape_a, BUFFER_SAMPLES);
        tape_move(&tape_b, BUFFER_SAMPLES);

        display_update();
    }

    printf("Quit\n");

    save_tape(&tape_a);
    save_tape(&tape_b);

    instinct_close();
    display_close();
    audio_close();
    tape_destroy(&tape_a);
    tape_destroy(&tape_b);
    return 0;
}

float mix(sample * in1, sample * in2, sample * in3, sample * out, int num_samples,
         bool enable1, bool enable2, bool enable3,
         float vol1, float vol2, float vol3) {
    float peak = 0;
    for (int i = 0; i < num_samples; i++) {
        sample mixed = 0;
        if (enable1)
            mixed += in1[i] * vol1;
        if (enable2)
            mixed += in2[i] * vol2;
        if (enable3)
            mixed += in3[i] * vol3;
        if (mixed > 1.0)
            mixed = 1.0;
        else if (mixed < -1.0)
            mixed = -1.0;
        if (mixed > peak)
            peak = mixed;
        else if (-mixed > peak)
            peak = -mixed;
        out[i] = mixed;
    }
    return peak;
}

unsigned int time_millis(void) {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

float linear_volume_to_exponential(float linear) {
    // https://www.dr-lex.be/info-stuff/volumecontrols.html
    return linear * linear * linear * linear;
}

float exponential_volume_to_linear(float exponential) {
    return pow(exponential, 0.25);
}

void beep(void) {
    beep_time = BEEP_FRAMES;
}

void beep_sample(sample * out, int num_samples) {
    for (int i = 0; i < num_samples; i += OUT_CHANNELS) {
        short isample = beep_time * 1486; // should be about 1000 Hz
        isample /= 6;
        sample fsample = isample / 32768.0;
        for (int c = 0; c < OUT_CHANNELS; c++) {
            out[i + c] = fsample;
        }
        beep_time--;
    }
}
