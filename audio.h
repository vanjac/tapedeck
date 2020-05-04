#include <pulse/simple.h>

// affects playback latency
#define PLAYBACK_BUFFER_SIZE 4096
// affects record latency
#define RECORD_BUFFER_SIZE 2048

int audio_open(void);
void audio_close(void);
int audio_read(uint8_t * buffer);
int audio_write(uint8_t * buffer);

