#include <stdbool.h>

#define BYTES_PER_SAMPLE 2
#define TAPE_CHANNELS 2
#define TAPE_BYTES_PER_FRAME (BYTES_PER_SAMPLE * TAPE_CHANNELS)
#define OUT_CHANNELS 2
#define OUT_BYTES_PER_FRAME (BYTES_PER_SAMPLE * OUT_CHANNELS)
#define FRAME_RATE 44100
#define BUFFER_SIZE (512 * TAPE_BYTES_PER_FRAME)
#define BEEP_SAMPLES 4096

bool quit_flag;

float audio_in_volume;
bool link_tapes;

float linear_volume_to_exponential(float linear);
float exponential_volume_to_linear(float exponential);

void beep(void);
unsigned int time_millis(void);

