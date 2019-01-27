#include <stdbool.h>

#define BUFFER_SIZE 1024
#define SAMPLE_BYTES 4

bool quit_flag;

float audio_in_volume;
bool link_tapes;
int beep_time;

void beep(void);
unsigned int time_millis(void);

