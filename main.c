#include <stdio.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "tape.h"
#include "instinct.h"

#define BUFFER_SIZE 1024
// affects playback latency
#define PLAYBACK_BUFFER_SIZE 4096
// affects record latency
#define RECORD_BUFFER_SIZE 2048

int main(int argc, char *argv[]) {
    if (instinct_open())
        return 1;

    static const pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    static const pa_buffer_attr playback_buff_attr = {
        .tlength = PLAYBACK_BUFFER_SIZE,
        .maxlength = -1,
        .minreq = -1,
        .prebuf = -1
    };

    static const pa_buffer_attr record_buff_attr = {
        .fragsize = RECORD_BUFFER_SIZE,
        .maxlength = -1
    };

    int pa_error;

    pa_simple *play_stream = pa_simple_new(
        NULL,               // default server
        "tapedeck",         // name of app
        PA_STREAM_PLAYBACK,
        NULL,               // default device
        "playback",         // stream description
        &sample_spec,
        NULL,               // default channel map
        &playback_buff_attr,
        &pa_error);
    if (!play_stream) {
        fprintf(stderr, "Couldn't open playback stream: %s\n", pa_strerror(pa_error));
        return 1;
    }
    pa_simple *record_stream = pa_simple_new(
        NULL,               // default server
        "tapedeck",         // name of app
        PA_STREAM_RECORD,
        NULL,               // default device
        "record",           // stream description
        &sample_spec,
        NULL,               // default channel map
        &record_buff_attr,
        &pa_error);
    if (!record_stream) {
        fprintf(stderr, "Couldn't open record stream: %s\n", pa_strerror(pa_error));
        return 1;
    }

    uint8_t audio_buffer[BUFFER_SIZE];

    while(1) {
        pa_simple_read(record_stream, audio_buffer, sizeof(audio_buffer), &pa_error);
        pa_simple_write(play_stream, audio_buffer, sizeof(audio_buffer), &pa_error);

        if (instinct_update()) {
            return 1;
        }
    }

    pa_simple_free(play_stream);
    pa_simple_free(record_stream);
    instinct_close();
    return 0;
}

