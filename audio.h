#include <pulse/simple.h>
#include "main.h"

// affects playback latency
#define PLAYBACK_BUFFER_SIZE (2048 * sizeof(sample))
// affects record latency
#define RECORD_BUFFER_SIZE (1024 * sizeof(sample))

int audio_open(void);
void audio_close(void);
int audio_read(sample * buffer, int num_samples);
int audio_write(sample * buffer, int num_samples);
int audio_write_aux(sample * buffer, int num_samples);
int audio_flush(void);
