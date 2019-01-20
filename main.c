#include "main.h"
#include "tape.h"
#include "instinct.h"
#include "audio.h"

int main(int argc, char *argv[]) {
    if (instinct_open())
        return 1;

    if (audio_open())
        return 1;

    uint8_t audio_buffer[BUFFER_SIZE];

    while(1) {
        if (audio_read(audio_buffer))
            return 1;

        if (instinct_update()) {
            return 1;
        }

        if (audio_write(audio_buffer))
            return 1;
    }

    audio_close();
    instinct_close();
    return 0;
}

