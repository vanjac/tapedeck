#include <stdbool.h>

#define BUFFER_SIZE 2048
#define SAMPLE_BYTES 4

bool quit_flag;

float audio_in_volume;
bool link_tapes;
int beep_time;

float linear_volume_to_exponential(float linear);
float exponential_volume_to_linear(float exponential);

void beep(void);
unsigned int time_millis(void);

