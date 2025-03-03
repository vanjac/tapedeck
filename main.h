#pragma once

#include <stdbool.h>

#define TAPE_CHANNELS 2
#define OUT_CHANNELS 2
#define FRAME_RATE 44100
#define BUFFER_FRAMES 512
#define BUFFER_SAMPLES (BUFFER_FRAMES * TAPE_CHANNELS)
#define BEEP_FRAMES 2048

typedef float sample;

extern bool quit_flag;

extern float audio_in_volume;
extern bool link_tapes;

float linear_volume_to_exponential(float linear);
float exponential_volume_to_linear(float exponential);

void beep(void);
unsigned int time_millis(void);

